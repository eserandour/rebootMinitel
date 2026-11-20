"""Exécute tous les exemples de tutoriel.md dans le simulateur, vérifie les limites de
l'Arduino Mega et produit sortie/tutoriel_final.md avec les images d'écran.

Marqueurs placés juste après un bloc ```basic :
  <!--ecran [options]-->   exécute le bloc et insère une image de l'écran
  <!--test [options]-->    exécute le bloc sans image
  (un bloc sans marqueur est exécuté comme un <!--test-->)
Options : mode=direct (lignes tapées telles quelles, sans RUN), mode=accueil (marqueur seul),
  keys="..." (touches après RUN), limit=N, rows=N, first=N, last=N, erreur (erreur attendue),
  pc (la passerelle est branchée, avec le dossier sortie/pc : SAVE "PC:NOM"...)
"""
import re, os, sys, shutil, shlex, socket, subprocess, threading
ICI = os.path.dirname(os.path.abspath(__file__))
SIMDIR = os.path.join(ICI, '..', 'simulateur')
sys.path.insert(0, SIMDIR)
import ecran
import tokens
sys.path.insert(0, os.path.join(ICI, '..', 'passerelle'))
from passerelle import Passerelle, servir

SORTIE = os.path.join(ICI, 'sortie')
IMG = os.path.join(SORTIE, 'img')
SD = os.path.join(SORTIE, 'carte_sd')
TMP = os.path.join(SORTIE, 'tmp')
PC = os.path.join(SORTIE, 'pc')
SIMULATEUR = os.path.join(SIMDIR, 'build', 'simulateur')

subprocess.run(['sh', os.path.join(SIMDIR, 'compiler.sh')], check=True)
for d in (IMG, SD, TMP, PC):
    shutil.rmtree(d, ignore_errors=True)
    os.makedirs(d)
# Les pages Vidéotex des exemples, sur la carte SD (pour VDT)
EXEMPLES = os.path.join(ICI, '..', '..', 'exemples')
for f in os.listdir(EXEMPLES):
    if f.upper().endswith('.VDT'):
        shutil.copy(os.path.join(EXEMPLES, f), os.path.join(SD, f))

lignes = open(os.path.join(ICI, 'tutoriel.md'), encoding='utf-8').read().split('\n')
sortie, problemes, rapport = [], [], []
n_ex = n_img = 0


def options(s):
    o = {}
    for t in shlex.split(s):
        k, _, v = t.partition('=')
        o[k] = v if _ else True
    return o


def attentes(touches):
    # Une petite attente avant chaque touche {k:x} qui n'en a pas déjà une
    res = []
    for t in re.findall(r'\{[^}]*\}|.', touches, re.S):
        if t.startswith('{k:') and not (res and res[-1].startswith('{WAIT:')):
            res.append('{WAIT:60}')
        res.append(t)
    return ''.join(res)


def verifier(code, nom):
    total = 0
    for l in code:
        if len(l) > 78:
            problemes.append(f'{nom} : ligne de {len(l)} caractères : {l}')
        for ch in re.findall(r'"([^"]*)"', l):
            if len(ch.encode('utf-8')) > 32:
                problemes.append(f'{nom} : chaîne de plus de 32 octets : "{ch}"')
        m = re.match(r'\s*\d+\s*(.*)$', l)
        if m:
            total += tokens.taille_ligne(m.group(1))  # Mots-clés tokenisés, comme sur la carte
    if total > tokens.MEMOIRE_MEGA:
        problemes.append(f'{nom} : programme de {total} octets')


class FluxPrise:
    # Une prise vue comme le port série de la passerelle (read renvoie None à la fin)
    def __init__(self, prise): self.prise = prise
    def read(self, n): return self.prise.recv(n) or None
    def write(self, b): self.prise.sendall(b)


def executer(script, nom, limite=None, pc=False):
    ft, fe = os.path.join(TMP, nom + '.txt'), os.path.join(TMP, nom + '.ecran')
    open(ft, 'w', encoding='utf-8').write(script)
    args = [SIMULATEUR, SD + '/', ft, fe] + ([str(limite)] if limite else [])
    env, fds = dict(os.environ), ()
    if pc:  # Le câble USB : une prise reliée à la passerelle, qui tourne à côté
        carte, prise = socket.socketpair()
        env['REBASIC_PC'], fds = str(carte.fileno()), (carte.fileno(),)
        threading.Thread(target=servir, args=(FluxPrise(prise), Passerelle(PC)), daemon=True).start()
    r = subprocess.run(args, capture_output=True, text=True, timeout=300, env=env, pass_fds=fds)
    if pc:
        carte.close()
    if r.returncode:
        problemes.append(f'{nom} : le simulateur a échoué\n{r.stderr[-2000:]}')
    erreurs, interruptions = map(int, open(fe + '.err').read().split())
    return fe, erreurs - interruptions


def inserer_image(fe, nom, o):
    ecran.image(fe, os.path.join(IMG, nom + '.png'), rangees=int(o['rows']) if 'rows' in o else None,
                premiere=int(o.get('first', 0)), dernieres=int(o['last']) if 'last' in o else None)
    sortie.append('')
    sortie.append(f'::: {{custom-style="Capture"}}\n![](img/{nom}.png){{width=10.5cm}}\n:::')


i = 0
while i < len(lignes):
    l = lignes[i]
    seul = re.match(r'<!--(ecran|test)\s*(.*?)-->\s*$', l)
    if seul:   # Marqueur sans bloc : l'écran d'accueil de reBASIC
        n_ex += 1
        nom = f'ex{n_ex:03d}'
        fe, err = executer('', nom)
        n_img += 1
        inserer_image(fe, nom, options(seul.group(2)))
        i += 1
        continue
    if l.strip() == '```basic':
        j = i + 1
        while lignes[j].strip() != '```':
            j += 1
        code = lignes[i + 1:j]
        m = re.match(r'<!--(ecran|test)\s*(.*?)-->\s*$', lignes[j + 1]) if j + 1 < len(lignes) else None
        n_ex += 1
        nom = f'ex{n_ex:03d}'
        o = options(m.group(2)) if m else {}
        genre = m.group(1) if m else 'test'
        touches = attentes(o.get('keys', '').encode().decode('unicode_escape').encode('latin-1').decode('utf-8'))
        if o.get('mode') == 'direct':
            verifier([c for c in code if re.match(r'\s*\d', c)], nom)
            script = '{CLR}' + '\n'.join(code) + '\n' + touches
        else:
            verifier(code, nom)
            script = 'NEW\n' + '\n'.join(code) + '\n' + ('{CLR}' if genre == 'ecran' else '') + 'RUN\n' + touches
        fe, err = executer(script, nom, int(o['limit']) if 'limit' in o else None, 'pc' in o)
        if err and 'erreur' not in o:
            problemes.append(f'{nom} : {err} erreur(s) inattendue(s)\n' + ecran.texte(fe)[0])
        if 'erreur' in o and not err:
            problemes.append(f'{nom} : une erreur était attendue')
        txt, infos = ecran.texte(fe)
        rapport.append(f'===== {nom} ({genre}) erreurs={err}\n' + '\n'.join(code) + '\n----- écran\n' + txt)
        sortie.extend(lignes[i:j + 1])
        if genre == 'ecran':
            n_img += 1
            inserer_image(fe, nom, o)
        i = j + 2 if m else j + 1
        continue
    sortie.append(l)
    i += 1

open(os.path.join(SORTIE, 'tutoriel_final.md'), 'w', encoding='utf-8').write('\n'.join(sortie))
open(os.path.join(SORTIE, 'rapport.txt'), 'w', encoding='utf-8').write('\n\n'.join(rapport))
shutil.rmtree(TMP, ignore_errors=True)
print(f'{n_ex} exemples exécutés, {n_img} images d\'écran')
if problemes:
    print('\n'.join(problemes))
    sys.exit(1)
print('Aucun problème.')
