"""Dessin de l'écran du Minitel enregistré par le simulateur (image PNG ou texte)."""
from PIL import Image, ImageDraw, ImageFont
import os

# Niveaux de gris du Minitel 1B pour les couleurs 0 à 7 (noir, rouge, vert, jaune, bleu, magenta, cyan, blanc)
GRIS = {0: 0, 4: 70, 1: 100, 5: 130, 2: 160, 6: 190, 3: 215, 7: 240}
LARGEUR, HAUTEUR = 14, 21   # Taille d'une case en pixels (un point semi-graphique : 7 x 7)
POLICES = ['/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf',
           '/Library/Fonts/DejaVuSansMono.ttf', 'C:/Windows/Fonts/consola.ttf']


def lire(fichier):
    cases, infos = {}, {}
    for ligne in open(fichier, encoding='utf-8', errors='replace'):
        p = ligne.rstrip('\n').split('\t')
        if p[0] in ('CURSEUR', 'BIPS', 'JOURNAL'):
            infos[p[0]] = p[1:]
            continue
        r, c, coul, inv, gfx, pts = map(int, p[:6])
        car = p[6] if len(p) > 6 and p[6] else ' '
        cases[(r, c)] = (coul, inv, gfx, pts, car)
    return cases, infos


def rangees_utilisees(cases):
    derniere = -1
    for (r, c), (coul, inv, gfx, pts, car) in cases.items():
        if car != ' ' or inv or (gfx and pts):
            derniere = max(derniere, r)
    return derniere + 1


def police():
    for f in POLICES:
        if os.path.exists(f):
            return ImageFont.truetype(f, 20)
    return ImageFont.load_default()


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
            coul, inv, gfx, pts, car = cases[(r, c)]
            x, y = m + c * LARGEUR, m + (r - premiere) * HAUTEUR
            g = GRIS[coul]
            if gfx:
                for bx, by, masque in [(0, 0, 32), (1, 0, 16), (0, 1, 8), (1, 1, 4), (0, 2, 2), (1, 2, 1)]:
                    if pts & masque:
                        d.rectangle([x + bx * 7, y + by * 7, x + bx * 7 + 6, y + by * 7 + 6], fill=(g, g, g))
                continue
            couleur_texte = (g, g, g)
            if inv:
                d.rectangle([x, y, x + LARGEUR - 1, y + HAUTEUR - 1], fill=(g, g, g))
                couleur_texte = (8, 8, 10)
            if car.strip():
                d.text((x + LARGEUR / 2, y + HAUTEUR / 2 + 1), car, font=f, fill=couleur_texte, anchor='mm')
    img.save(sortie)


def texte(fichier, premiere=0):
    """L'écran en texte : # pour un point semi-graphique allumé."""
    cases, infos = lire(fichier)
    lignes = []
    for r in range(premiere, rangees_utilisees(cases)):
        s = ''
        for c in range(40):
            coul, inv, gfx, pts, car = cases[(r, c)]
            s += ('#' if pts else ' ') if gfx else car
        lignes.append(s.rstrip())
    return '\n'.join(lignes), infos
