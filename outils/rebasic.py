#!/usr/bin/env python3
"""Simulateur de reBASIC : exécuter des programmes sur un ordinateur, sans Minitel.

  python3 rebasic.py lancer PROGRAMME.bas [options]   exécute un programme
  python3 rebasic.py interactif [PROGRAMME.bas]       utilise reBASIC dans le terminal
  python3 rebasic.py script TOUCHES.txt [options]     rejoue une suite de touches
  python3 rebasic.py compiler                         recompile le simulateur
"""
import argparse, os, subprocess, sys, tempfile

ICI = os.path.dirname(os.path.abspath(__file__))
SIMULATEUR = os.path.join(ICI, 'build', 'simulateur')
BASIC = os.path.join(ICI, '..', '..', 'Basic.ino')
sys.path.insert(0, ICI)
import ecran


def compiler(forcer=False):
    a_jour = os.path.exists(SIMULATEUR) and os.path.getmtime(SIMULATEUR) >= max(
        os.path.getmtime(BASIC), *(os.path.getmtime(os.path.join(ICI, f)) for f in ('simulateur.cpp', 'imitations.h')))
    if forcer or not a_jour:
        subprocess.run(['sh', os.path.join(ICI, 'compiler.sh')], check=True)


def executer(touches, sd, limite=None, interactif=False):
    """Lance le simulateur avec une suite de touches. Renvoie (fichier écran, erreurs, interruptions)."""
    compiler()
    os.makedirs(sd, exist_ok=True)
    tmp = tempfile.mkdtemp(prefix='rebasic_')
    f_touches = os.path.join(tmp, 'touches.txt')
    f_ecran = os.path.join(tmp, 'ecran.txt')
    open(f_touches, 'w', encoding='utf-8').write(touches)
    args = [SIMULATEUR, os.path.abspath(sd) + '/', f_touches, f_ecran]
    if limite:
        args.append(str(limite))
    if interactif:
        args.append('--interactif')
    subprocess.run(args, check=True)
    erreurs, interruptions = map(int, open(f_ecran + '.err').read().split())
    return f_ecran, erreurs, interruptions


def lignes_programme(fichier):
    lignes = [l.rstrip('\r\n') for l in open(fichier, encoding='utf-8')]
    return [l for l in lignes if l.strip()]


def verifier(lignes):
    """Contrôle les limites de l'Arduino Mega. Renvoie une liste d'avertissements."""
    import re
    avert, total = [], 0
    for l in lignes:
        if len(l) > 78:
            avert.append(f'ligne de plus de 78 caractères : {l}')
        for chaine in re.findall(r'"([^"]*)"', l):
            if len(chaine.encode('utf-8')) > 32:
                avert.append(f'chaîne de plus de 32 octets (tronquée sur le Mega) : "{chaine}"')
        m = re.match(r'\s*\d+\s*(.*)$', l)
        if m:
            total += 3 + len(m.group(1).rstrip().encode('utf-8')) + 1
    if total > 2048:
        avert.append(f'programme de {total} octets (2048 au plus sur le Mega)')
    return avert


def montrer(f_ecran, erreurs, interruptions, options):
    txt, infos = ecran.texte(f_ecran)
    print(txt)
    journal = ' '.join(infos.get('JOURNAL', [''])).strip()
    if journal:
        print('\nJournal (son, broches) :', journal[:2000])
    if interruptions:
        print(f'\n(programme interrompu par le simulateur après {options.limite or 400000} attentes : boucle sans fin ?)')
    if erreurs - interruptions > 0:
        print(f'\n{erreurs - interruptions} erreur(s) affichée(s) par reBASIC')
    if options.png:
        ecran.image(f_ecran, options.png, rangees=options.rangees, premiere=options.premiere, dernieres=options.dernieres)
        print('\nImage :', options.png)


def main():
    p = argparse.ArgumentParser(description='Simulateur de reBASIC', formatter_class=argparse.RawDescriptionHelpFormatter, epilog=__doc__)
    sous = p.add_subparsers(dest='commande', required=True)

    def options_communes(sp):
        sp.add_argument('--sd', default=os.path.join(ICI, 'carte_sd'), help='dossier qui joue le rôle de la carte SD')
        sp.add_argument('--limite', type=int, help='attentes avant d\'interrompre une boucle sans fin (400000 par défaut)')
        sp.add_argument('--png', help='enregistre une image de l\'écran')
        sp.add_argument('--rangees', type=int, help='nombre de rangées de l\'image (24 pour l\'écran entier)')
        sp.add_argument('--premiere', type=int, default=0, help='première rangée de l\'image (à partir de 0)')
        sp.add_argument('--dernieres', type=int, help='seulement les N dernières rangées utilisées')

    sp = sous.add_parser('lancer', help='exécute un programme (fichier .bas)')
    sp.add_argument('programme')
    sp.add_argument('-t', '--touches', default='', help='touches tapées après RUN, ex. "12\\n" ou "{WAIT:500}{k:a}"')
    sp.add_argument('--avec-saisie', action='store_true', help='montre aussi le programme tapé avant RUN')
    options_communes(sp)

    sp = sous.add_parser('script', help='rejoue un fichier de touches')
    sp.add_argument('touches')
    options_communes(sp)

    sp = sous.add_parser('interactif', help='utilise reBASIC dans le terminal')
    sp.add_argument('programme', nargs='?')
    sp.add_argument('--sd', default=os.path.join(ICI, 'carte_sd'))

    sous.add_parser('compiler', help='recompile le simulateur')
    o = p.parse_args()

    if o.commande == 'compiler':
        compiler(forcer=True)
    elif o.commande == 'lancer':
        lignes = lignes_programme(o.programme)
        for a in verifier(lignes):
            print('Attention :', a)
        touches = o.touches.encode('utf-8').decode('unicode_escape').encode('latin-1').decode('utf-8')
        script = 'NEW\n' + '\n'.join(lignes) + '\n' + ('' if o.avec_saisie else '{CLR}') + 'RUN\n' + touches
        montrer(*executer(script, o.sd, o.limite), o)
    elif o.commande == 'script':
        montrer(*executer(open(o.touches, encoding='utf-8').read(), o.sd, o.limite), o)
    elif o.commande == 'interactif':
        script = ''
        if o.programme:
            script = 'NEW\n' + '\n'.join(lignes_programme(o.programme)) + '\n{CLR}'
        compiler()
        os.makedirs(o.sd, exist_ok=True)
        tmp = tempfile.mkdtemp(prefix='rebasic_')
        f = os.path.join(tmp, 'touches.txt')
        open(f, 'w', encoding='utf-8').write(script)
        subprocess.run([SIMULATEUR, os.path.abspath(o.sd) + '/', f, '', '--interactif'])


if __name__ == '__main__':
    main()
