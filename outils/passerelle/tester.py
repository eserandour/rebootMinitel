#!/usr/bin/env python3
"""
Test de la passerelle avec le simulateur de reBASIC (le vrai Basic.ino, compilé sur PC) :
SAVE, LOAD, MERGE et FILES "PC:", fichier introuvable, PC absent, l'heure (@HEURE),
et la copie de fichiers du menu 3 (@ECRIRE, @LIRE).

Usage : python3 tester.py   (Linux ou macOS ; compile le simulateur si besoin)
"""

import os
import re
import shutil
import socket
import subprocess
import sys
import tempfile
import threading

ICI = os.path.dirname(os.path.abspath(__file__))
SIMU = os.path.join(ICI, '..', 'simulateur')
sys.path.insert(0, ICI)
from passerelle import Passerelle, servir  # noqa: E402


class FluxPrise:
    def __init__(self, prise): self.prise = prise
    def read(self, n):
        d = self.prise.recv(n)
        return d if d else None
    def write(self, b): self.prise.sendall(b)


def ecran(fichier):
    # Le texte de l'écran simulé, rangée par rangée
    cases = {}
    for l in open(fichier, encoding='utf-8'):
        c = l.rstrip('\n').split('\t')
        if len(c) == 12 and c[0].isdigit():
            cases[(int(c[0]), int(c[1]))] = ' ' if c[10] == '1' else c[11]
    return [''.join(cases[(r, j)] for j in range(40)).rstrip() for r in range(24)]


def lancer(touches, dossier_pc):
    """Lance le simulateur avec ces touches ; dossier_pc None : pas de passerelle branchée."""
    tmp = tempfile.mkdtemp()
    sd = os.path.join(tmp, 'sd'); os.mkdir(sd)
    with open(os.path.join(tmp, 'touches'), 'w', encoding='utf-8') as f:
        f.write(touches)
    env = dict(os.environ)
    fds = ()
    if dossier_pc is not None:
        carte, pc = socket.socketpair()
        env['REBASIC_PC'] = str(carte.fileno())
        fds = (carte.fileno(),)
        fil = threading.Thread(target=servir, args=(FluxPrise(pc), Passerelle(dossier_pc)), daemon=True)
        fil.start()
    subprocess.run([os.path.join(SIMU, 'build', 'simulateur'), sd, os.path.join(tmp, 'touches'),
                    os.path.join(tmp, 'ecran')], env=env, pass_fds=fds, check=True, timeout=60)
    if dossier_pc is not None:
        carte.close()
    texte = ecran(os.path.join(tmp, 'ecran'))
    shutil.rmtree(tmp)
    return texte


def verifier(nom, condition, texte):
    print(('ok    ' if condition else 'ÉCHEC ') + nom)
    if not condition:
        print('\n'.join('   |' + l for l in texte))
    return condition


def main():
    if not os.path.exists(os.path.join(SIMU, 'build', 'simulateur')):
        subprocess.run(['sh', os.path.join(SIMU, 'compiler.sh')], check=True)
    pc = tempfile.mkdtemp()
    bon = True

    # 1. SAVE "PC:" puis FILES "PC:"
    t = lancer('10 PRINT "Bonjour éèà"\n20 FOR I=1 TO 3:PRINT I:NEXT\nSAVE "pc:essai"\nFILES "PC:"\n', pc)
    contenu = open(os.path.join(pc, 'ESSAI.BAS'), 'rb').read()
    bon &= verifier('SAVE "PC:ESSAI" écrit ESSAI.BAS',
                    contenu == '10 PRINT "Bonjour éèà"\r\n20 FOR I=1 TO 3:PRINT I:NEXT\r\n'.encode('utf-8'), t)
    bon &= verifier('FILES "PC:" liste ESSAI et sa taille', any(l.startswith('ESSAI        ' + str(len(contenu))) for l in t), t)

    # 2. LOAD "PC:" d'un fichier écrit sur le PC (minuscules, BOM, CR LF, ligne vide)
    with open(os.path.join(pc, 'jeu.bas'), 'wb') as f:
        f.write(b'\xef\xbb\xbf10 print "Charge depuis le PC"\r\n\r\n30 end\r\n')
    t = lancer('LOAD "PC:JEU"\nLIST\nRUN\n', pc)
    bon &= verifier('LOAD "PC:JEU" charge et exécute', '10 PRINT "Charge depuis le PC"' in t and '30 END' in t
                    and 'Charge depuis le PC' in t, t)

    # 3. MERGE "PC:"
    t = lancer('20 PRINT "garde"\n30 PRINT "remplacée"\nMERGE "PC:JEU"\nLIST\n', pc)
    bon &= verifier('MERGE "PC:JEU" ajoute et remplace', '20 PRINT "garde"' in t and '30 END' in t
                    and '30 PRINT "remplacée"' not in t[-6:], t)

    # 4. Gros programme (plus que le tampon de 64 octets de l'Arduino Mega)
    lignes = ''.join('%d PRINT "Ligne numero %d avec du texte pour remplir"\n' % (i * 10, i) for i in range(1, 30))
    t = lancer(lignes + 'SAVE "PC:GROS"\nNEW\nLOAD "PC:GROS"\nLIST 280-\n', pc)
    bon &= verifier('Aller-retour d\'un programme de 29 lignes', any('290 PRINT "Ligne numero 29' in l for l in t)
                    and open(os.path.join(pc, 'GROS.BAS'), 'rb').read().count(b'\r\n') == 29, t)

    # 5. Fichier introuvable, nom incorrect
    t = lancer('LOAD "PC:ABSENT"\nSAVE "PC:BEAUCOUPTROPLONG"\n', pc)
    bon &= verifier('LOAD d\'un fichier absent', 'Fichier introuvable' in t, t)
    bon &= verifier('Nom trop long refusé', 'Valeur incorrecte' in t, t)

    # 6. Pas de passerelle : PC absent
    t = lancer('10 END\nSAVE "PC:X"\nFILES "PC:"\n', None)
    bon &= verifier('Sans passerelle : PC absent', sum('PC absent' in l for l in t) == 2, t)

    # 7. La carte SD marche toujours, et "PC:" est refusé là où il n'a pas de sens
    t = lancer('10 END\nSAVE "SD1"\nFILES\nOPEN "PC:A" FOR OUTPUT AS #1\nFILES "SD:"\n', pc)
    bon &= verifier('SAVE sur la carte SD inchangé', any(l.startswith('SD1') for l in t), t)
    bon &= verifier('OPEN "PC:..." et FILES "SD:" refusés', sum('Valeur incorrecte' in l for l in t) == 2, t)

    # 8. L'heure, demandée par une carte sans horloge RTC, même au milieu d'un LOAD
    p = Passerelle(pc)
    p.recevoir(b'@LOAD ESSAI.BAS')
    r = p.recevoir(b'@HEURE')
    bon &= verifier('@HEURE : date et heure de l\'ordinateur', len(r) == 1 and re.fullmatch(rb'@HEURE \d\d/\d\d/\d{4} \d\d:\d\d:\d\d', r[0]) is not None, [repr(r)])
    bon &= verifier('@HEURE n\'interrompt pas un LOAD', p.recevoir(b'@SUITE')[0].startswith(b'10 PRINT'), [])

    # 9. Copie de fichiers quelconques (menu 3) : @ECRIRE et @LIRE, en hexadécimal
    octets = bytes(range(256)) * 3 + b'\x1b\x0c\r\n'
    p = Passerelle(pc)
    reponses = p.recevoir(b'@ECRIRE page.vdt')
    texte = octets.hex().upper().encode('ascii')
    for i in range(0, len(texte), 64):
        reponses += p.recevoir(texte[i:i + 64])
    reponses += p.recevoir(b'@FIN')
    bon &= verifier('@ECRIRE : copie à l\'octet près', reponses == [b'@OK', b'@OK']
                    and open(os.path.join(pc, 'PAGE.VDT'), 'rb').read() == octets, [repr(reponses)])
    relu, lignes = b'', 0
    if p.recevoir(b'@LIRE PAGE.VDT') == [b'@OK']:
        while True:
            r = p.recevoir(b'@SUITE')[0]
            if r == b'@FIN':
                break
            relu += bytes.fromhex(r.decode('ascii'))
            lignes += 1
    bon &= verifier('@LIRE : 32 octets par ligne, à l\'octet près', relu == octets and lignes == (len(octets) + 31) // 32, [])
    p.recevoir(b'@ECRIRE ABIME.VDT')
    p.recevoir(b'4G')
    bon &= verifier('@ECRIRE : données abîmées refusées', p.recevoir(b'@FIN') == [b'@ERR DONNEES']
                    and not os.path.exists(os.path.join(pc, 'ABIME.VDT')), [])

    shutil.rmtree(pc)
    print('Tous les tests sont passés.' if bon else 'Des tests ont échoué.')
    sys.exit(0 if bon else 1)


if __name__ == '__main__':
    main()
