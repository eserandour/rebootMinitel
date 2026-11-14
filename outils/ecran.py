"""Dessin de l'écran du Minitel enregistré par le simulateur (image PNG ou texte)."""
from PIL import Image, ImageDraw, ImageFont
import os

# Niveaux de gris du Minitel 1B pour les couleurs 0 à 7 (noir, rouge, vert, jaune, bleu, magenta, cyan, blanc)
GRIS = {0: 0, 4: 70, 1: 100, 5: 130, 2: 160, 6: 190, 3: 215, 7: 240}
# Une case fait 16 x 20 pixels : les proportions de la matrice 8 x 10 du Minitel.
# Un point semi-graphique fait donc 8 pixels de large et environ 6,7 de haut.
LARGEUR, HAUTEUR = 16, 20
RANGS = [(0, 7), (7, 13), (13, 20)]   # Les 3 rangées de points d'une case (début, fin) en pixels
POLICES = ['/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf',
           '/Library/Fonts/DejaVuSansMono.ttf', 'C:/Windows/Fonts/consola.ttf']


def lire(fichier):
    cases, infos = {}, {}
    for ligne in open(fichier, encoding='utf-8', errors='replace'):
        p = ligne.rstrip('\n').split('\t')
        if p[0] in ('CURSEUR', 'BIPS', 'JOURNAL'):
            infos[p[0]] = p[1:]
            continue
        v = list(map(int, p[:11]))
        car = p[11] if len(p) > 11 and p[11] else ' '
        cases[(v[0], v[1])] = dict(couleur=v[2], inverse=v[3], graphique=v[4], points=v[5], fond=v[6],
                                   souligne=v[7], clignote=v[8], taille=v[9], couverte=v[10], car=car)
    return cases, infos


def utilisee(x):
    return x['car'] != ' ' or x['inverse'] or x['fond'] or x['souligne'] or (x['graphique'] and x['points'])


def rangees_utilisees(cases):
    derniere = -1
    for (r, c), x in cases.items():
        if utilisee(x):
            derniere = max(derniere, r)
    return derniere + 1


_polices = {}


def police(taille=21):
    if taille in _polices:
        return _polices[taille]
    for f in POLICES:
        if os.path.exists(f):
            _polices[taille] = ImageFont.truetype(f, taille)
            return _polices[taille]
    return ImageFont.load_default()


def gris(n):
    g = GRIS[n]
    return (g, g, g)


def image(fichier, sortie, rangees=None, premiere=0, dernieres=None):
    """Dessine l'écran en PNG. Par défaut, seulement les rangées utilisées."""
    cases, infos = lire(fichier)
    if dernieres:
        premiere = max(0, rangees_utilisees(cases) - dernieres)
        rangees = dernieres
    n = rangees if rangees else max(1, rangees_utilisees(cases) - premiere)
    m = 10
    img = Image.new('RGB', (40 * LARGEUR + 2 * m, n * HAUTEUR + 2 * m), (28, 28, 30))
    d = ImageDraw.Draw(img)
    d.rectangle([m - 4, m - 4, 40 * LARGEUR + m + 3, n * HAUTEUR + m + 3], fill=(8, 8, 10))
    f = police()
    for r in range(premiere, premiere + n):
        for c in range(40):
            x = cases[(r, c)]
            if x['couverte']:
                continue   # Dessinée avec la case du grand caractère
            px, py = m + c * LARGEUR, m + (r - premiere) * HAUTEUR
            avant, fond = gris(x['couleur']), gris(x['fond'])
            if x['graphique']:
                d.rectangle([px, py, px + LARGEUR - 1, py + HAUTEUR - 1], fill=fond)
                for bx in range(2):
                    for by, (y0, y1) in enumerate(RANGS):
                        if x['points'] & (32 >> (by * 2 + bx)):
                            d.rectangle([px + bx * 8, py + y0, px + bx * 8 + 7, py + y1 - 1], fill=avant)
                continue
            if x['inverse']:
                avant, fond = fond, avant
            larg = 2 if x['taille'] & 2 else 1
            haut = 2 if x['taille'] & 1 else 1
            # Un grand caractère est dessiné vers le haut et vers la droite à partir de sa case
            w, h = LARGEUR * larg, HAUTEUR * haut
            top = py - HAUTEUR * (haut - 1)
            if r - (haut - 1) < premiere:   # Partie haute hors de l'image
                top = py
            # Le caractère est dessiné à haute résolution, dans les proportions de la matrice
            # 8 x 10 du Minitel, puis réduit à la taille de son emplacement
            grand = Image.new('RGB', (54, 80), fond)
            dg = ImageDraw.Draw(grand)
            if x['car'].strip():
                dg.text((27, 41), x['car'], font=police(66), fill=avant, anchor='mm')
            grand = grand.resize((LARGEUR * 4, HAUTEUR * 4), Image.LANCZOS)
            if x['souligne']:
                ImageDraw.Draw(grand).rectangle([0, HAUTEUR * 4 - 7, LARGEUR * 4 - 1, HAUTEUR * 4 - 1], fill=avant)
            case = grand.resize((w, h), Image.LANCZOS)
            img.paste(case, (px, top))
    img.save(sortie)


def texte(fichier, premiere=0):
    """L'écran en texte : # pour un point semi-graphique allumé."""
    cases, infos = lire(fichier)
    lignes = []
    for r in range(premiere, rangees_utilisees(cases)):
        s = ''
        for c in range(40):
            x = cases[(r, c)]
            if x['couverte']:
                s += ' '
            elif x['graphique']:
                s += '#' if x['points'] else ' '
            else:
                s += x['car']
        lignes.append(s.rstrip())
    return '\n'.join(lignes), infos
