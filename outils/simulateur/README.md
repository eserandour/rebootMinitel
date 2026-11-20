# Simulateur de reBASIC

Exécuter reBASIC sur un ordinateur, sans Minitel ni Arduino : pour essayer un programme, le déboguer, ou fabriquer des images d'écran.

Le simulateur compile le **vrai `Basic.ino`** du croquis avec un compilateur C++ ordinaire. Ce n'est pas une imitation de reBASIC : c'est son code qui exécute vos programmes. Seul le matériel est remplacé (`imitations.h`) :

| Matériel | Dans le simulateur |
|---|---|
| Minitel | un écran de 40 × 24 cases en mode rouleau : texte, couleurs, fond, vidéo inverse, soulignement, clignotement, double hauteur et largeur, semi-graphiques, défilement. Les règles sont celles du STUM 1B : fond et soulignement validés par un espace (attributs de zone), double hauteur dessinée vers le haut et sans effet sur la rangée 1, taille, inversion et soulignement annulés par le semi-graphique |
| Clavier | un script de touches, ou le terminal en mode interactif |
| Horloge DS3231 | le 14/11/2026 à 10:30:00 au lancement, puis le temps simulé |
| Temps (`millis`) | avance d'une milliseconde à chaque instruction : `PAUSE`, `TIMER`, `TIME$` restent cohérents |
| Carte SD | un dossier de l'ordinateur (`carte_sd/` par défaut) |
| Liaison avec le PC (`Serial`) | une prise reliée à la passerelle, pour `tester.py` de [`passerelle/`](../passerelle/) ; sinon rien n'est branché (`SAVE "PC:…"` répond `PC absent`) |
| Buzzer, broches | les appels sont notés dans un journal ; `DREAD` et `AREAD` renvoient des valeurs qui varient avec le temps |

Les tailles sont celles de l'**Arduino Mega** (2304 octets pour le programme et les tableaux, chaînes de 32 caractères…).

## Ce qu'il faut

- un compilateur C++17 (`g++` ou `clang++`) ;
- Python 3 et [Pillow](https://pypi.org/project/pillow/) (`pip install pillow`) pour les images.

Testé sous Linux. Sous macOS, il suffit des outils en ligne de commande de Xcode ; sous Windows, utilisez WSL.

## Utilisation

Le simulateur se compile tout seul au premier lancement, et chaque fois que `Basic.ino` a changé.

### Exécuter un programme

```sh
python3 rebasic.py lancer ../../exemples/MYSTERE.BAS -t "50\n25\n"
```

L'écran final s'affiche dans le terminal. Le programme est lu dans un fichier texte (une ligne numérotée par ligne de fichier) ; il est vérifié avant l'exécution : lignes de plus de 78 caractères, chaînes de plus de 32 octets, programme de plus de 2304 octets (mots-clés comptés comme sur la carte, voir `tokens.py`).

| Option | Effet |
|---|---|
| `-t "…"` ou `--touches "…"` | touches tapées après `RUN` (voir plus bas) |
| `--png ecran.png` | enregistre une image de l'écran |
| `--rangees 24` | nombre de rangées de l'image (par défaut, celles qui sont utilisées) |
| `--premiere N`, `--dernieres N` | cadrage de l'image |
| `--sd DOSSIER` | dossier qui joue le rôle de la carte SD |
| `--limite N` | attentes avant d'interrompre une boucle sans fin (400 000 par défaut) |
| `--avec-saisie` | montre aussi la frappe du programme avant `RUN` |

Un programme qui ne s'arrête jamais (`GOTO 60` sur lui-même, horloge, jeu…) est interrompu au bout de la limite, et l'écran est capturé juste avant l'interruption :

```sh
python3 rebasic.py lancer ../../exemples/BALLE.BAS --rangees 24 --png balle.png
```

### Utiliser reBASIC dans le terminal

```sh
python3 rebasic.py interactif
python3 rebasic.py interactif ../../exemples/PENDU.BAS    # avec un programme déjà chargé
```

L'écran du Minitel est redessiné dans le terminal à chaque fois que reBASIC attend une ligne (les semi-graphiques utilisent les caractères Unicode « sextants », qu'affichent la plupart des terminaux récents). Tapez une ligne puis Entrée. `Ctrl-D` ou `{SOMM}` quitte.

Limite du mode interactif : le clavier n'est lu qu'au moment où reBASIC attend une ligne. Un programme qui utilise `INKEY$` ne reçoit donc pas les touches pendant qu'il tourne ; pour le tester, utilisez un script de touches avec `{k:x}`.

### Rejouer une suite de touches

```sh
python3 rebasic.py script mes_touches.txt --png ecran.png
```

Le fichier contient exactement ce que l'on taperait au clavier du Minitel.

## Écrire les touches

| Écriture | Touche |
|---|---|
| texte ordinaire | tapé quand reBASIC attend une ligne (`INPUT`, invite `Ok`) |
| `\n` (dans `-t`) ou un retour à la ligne | **ENVOI** |
| `{ENVOI}` `{ANNUL}` `{SOMM}` `{SUITE}` `{RETOUR}` | touches de fonction, frappées à tout moment |
| `{CORR}` `{GUIDE}` | CORRECTION et GUIDE, frappées quand reBASIC attend une ligne |
| `{k:x}` | la touche `x`, frappée même pendant l'exécution (pour `INKEY$`) |
| `{WAIT:n}` | `n` millisecondes sans toucher au clavier |
| `{CLR}` | efface l'écran simulé (pour une image plus nette) |
| `{SNAP}` | capture l'écran à cet instant |

Exemple : `-t "{WAIT:500}{k:6}{WAIT:200}{k:6}{WAIT:200}{k: }"` frappe 6, puis 6, puis ESPACE, à intervalles réguliers.

## Fichiers

| Fichier | Rôle |
|---|---|
| `rebasic.py` | la commande à utiliser |
| `compiler.sh` | compile le simulateur (`./compiler.sh --verifier` ajoute les contrôles mémoire d'AddressSanitizer) |
| `simulateur.cpp` | programme principal : clavier scripté, mode interactif, capture de l'écran |
| `imitations.h` | imitations de l'Arduino, du Minitel, de la carte SD et de l'horloge |
| `tokens.py` | la place qu'occupe un programme dans la mémoire (mots-clés tokenisés), pour les vérifications |
| `ecran.py` | dessin de l'écran en PNG ou en texte |

Les dossiers `build/` et `carte_sd/` sont créés à l'utilisation.

## Fidélité

Ce qui s'affiche, et où, est exact : c'est le code de reBASIC qui tourne, et l'écran simulé suit les règles du Minitel (curseur, retour à la ligne, défilement). En revanche, la police n'est pas celle du Minitel (les cases ont toutefois ses proportions, 8 × 10), les niveaux de gris sont approximatifs, le clignotement est montré fixe dans les images (il clignote en mode interactif, si le terminal le permet), et la vitesse est simulée (une instruction par milliseconde ; le vrai Mega est un peu plus rapide ou plus lent selon les instructions).
