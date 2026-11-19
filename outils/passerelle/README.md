# Passerelle avec le PC

Enregistrer un programme reBASIC sur l'ordinateur, ou l'ouvrir depuis l'ordinateur, **directement depuis le Minitel**, sans passer par la carte SD.

La carte (Arduino Mega ou ESP32) reste reliée à l'ordinateur par son câble USB. Sur l'ordinateur, `passerelle.py` répond aux demandes de reBASIC et range les programmes dans un dossier. Sur le Minitel, il suffit d'ajouter `PC:` devant le nom :

| Sur le Minitel | Effet |
|---|---|
| `SAVE "PC:JEU"` | Enregistre le programme dans `JEU.BAS`, dans le dossier de l'ordinateur |
| `LOAD "PC:JEU"` | Charge `JEU.BAS` depuis l'ordinateur (remplace le programme en cours) |
| `MERGE "PC:JEU"` | Ajoute les lignes de `JEU.BAS` au programme en cours |
| `FILES "PC:"` | Liste les fichiers du dossier, avec leur taille |

Les fichiers ont le même format que sur la carte SD : on peut les écrire ou les corriger avec n'importe quel éditeur de texte, ou les ouvrir dans le [Minitel virtuel](../minitel-virtuel/). Les noms suivent le format 8.3 (1 à 8 lettres, chiffres, `_` ou `-`) ; les minuscules sont acceptées sur l'ordinateur (`jeu.bas` se charge avec `LOAD "PC:JEU"`).

Le menu 3 (« Carte SD ») s'en sert aussi pour **copier n'importe quel fichier** (programmes, pages Vidéotex, fichiers de données) entre la carte SD et l'ordinateur (voir [Copier des fichiers](#copier-des-fichiers-menu-3)).

La passerelle donne aussi **l'heure de l'ordinateur** à une carte qui n'a pas d'horloge RTC (voir [L'heure du PC](#lheure-du-pc)).

Comme `LOAD` et `SAVE` sur la carte SD, `"PC:"` marche aussi dans un programme (`100 SAVE "PC:SCORES"`). Sans carte SD (sur un ESP32 seul, par exemple), c'est le moyen de garder ses programmes.

## Ce qu'il faut

- Python 3 et la bibliothèque [pyserial](https://pypi.org/project/pyserial/) :

  ```
  python3 -m pip install pyserial
  ```

  (Sous Windows : `py -m pip install pyserial`.)
- Le croquis rebootMinitel à partir de la version 2026.11.19b (2026.11.19c pour l'heure, 2026.11.19d pour la copie du menu 3).

## Utilisation

1. Branchez la carte à l'ordinateur par son câble USB, et **fermez le moniteur série de l'IDE Arduino** (un seul programme à la fois peut utiliser le port).
2. Lancez la passerelle en lui donnant le dossier des programmes :

   ```
   python3 passerelle.py --dossier ~/Documents/reBASIC
   ```

   Sous Windows : `py passerelle.py --dossier C:\Users\moi\Documents\reBASIC`.
3. Sur le Minitel, dans reBASIC : `SAVE "PC:BONJOUR"`, `FILES "PC:"`, `LOAD "PC:BONJOUR"`…

La passerelle trouve toute seule la carte si un seul port USB série est branché. Sinon, elle affiche la liste des ports et il faut préciser le bon :

| Système | Exemple |
|---|---|
| Windows | `--port COM3` |
| Linux | `--port /dev/ttyACM0` (Mega), `--port /dev/ttyUSB0` (ESP32) |
| macOS | `--port /dev/cu.usbmodem14101` (Mega), `--port /dev/cu.usbserial-0001` (ESP32) |

La passerelle note chaque échange (`Enregistré : BONJOUR.BAS (12 lignes)`) et affiche aussi les messages que la carte envoie au démarrage (`Carte : Horloge introuvable`). On l'arrête avec Ctrl-C. Si la carte est débranchée, elle attend qu'on la rebranche.

| Option | Effet |
|---|---|
| `--dossier DOSSIER` | Dossier des programmes (par défaut : le dossier courant) |
| `--port PORT` | Port série de la carte |
| `--vitesse N` | Vitesse en bauds (9600 par défaut, comme `Serial.begin` dans `rebootMinitel.ino`) |

**Attention : l'Arduino Mega redémarre quand la passerelle ouvre le port.** C'est le fonctionnement normal de l'Arduino (la même chose arrive quand on ouvre le moniteur série). Lancez donc la passerelle **avant** de taper un programme, et laissez-la tourner. L'ESP32, lui, ne redémarre en général pas.

## Copier des fichiers (menu 3)

Dans le gestionnaire de la carte SD (menu 3 de rebootMinitel) :

- **de la carte vers le PC** : on choisit un fichier, puis `4 - Copier sur le PC`. Le fichier de même nom dans le dossier du PC est remplacé. Le nom doit suivre le format 8.3 (sur l'ESP32, la carte SD accepte des noms plus longs : ils sont refusés) ;
- **du PC vers la carte** : depuis la liste, `ENVOI` affiche les fichiers du dossier du PC ; on tape une lettre, puis `ENVOI` pour copier le fichier sur la carte. Un fichier de même nom sur la carte est remplacé, après un avertissement.

Les octets passent tels quels, quel que soit le fichier : une page Vidéotex arrive identique. La copie prend environ 2 secondes par kilo-octet (9600 bauds, octets envoyés en hexadécimal) ; le nombre d'octets copiés s'affiche chaque kilo-octet. Si la copie vers la carte échoue en route, le fichier à moitié copié est effacé.

## L'heure du PC

Sans horloge DS3231 (shield sans horloge, ESP32 seul), rebootMinitel garde l'heure avec une **horloge logicielle**, qui compte à partir de l'heure qu'on lui a donnée. Il la demande à la passerelle :

- au démarrage de la carte (l'Arduino Mega redémarre justement quand la passerelle ouvre le port) ;
- depuis l'écran d'accueil, toutes les 5 secondes tant qu'il n'a pas l'heure : il suffit de lancer la passerelle, l'heure apparaît en bas de l'écran ;
- puis toutes les 10 minutes, toujours depuis l'écran d'accueil, pour corriger la dérive de l'horloge logicielle (le quartz du Mega peut dériver de quelques secondes par heure) ;
- dans reBASIC, si `TIME$`, `DATE$`, `HOUR`… sont demandés alors que l'heure n'est pas encore connue.

Sans passerelle, on peut régler l'horloge logicielle à la main (menu 0). Dans tous les cas, elle perd l'heure quand la carte s'éteint ou redémarre.

Avec une horloge DS3231, c'est elle qui donne l'heure, et rien n'est demandé au PC. Une exception : si l'horloge a perdu l'heure (pile vide), elle prend celle du PC au démarrage, plutôt que la date de compilation du croquis.

## Messages sur le Minitel

| Message | Cause |
|---|---|
| PC absent (dans le menu 3 : « PC absent : la passerelle tourne ? ») | La passerelle ne répond pas (pas lancée, port occupé par le moniteur série, câble débranché…), au bout de 3 secondes |
| Fichier introuvable | Pas de fichier de ce nom dans le dossier |
| Erreur de fichier | La passerelle n'a pas pu lire ou écrire le fichier (droits du dossier, disque plein…) |
| Valeur incorrecte | Nom qui ne suit pas le format 8.3 |

Le Minitel virtuel n'est relié à aucune carte : il répond `PC absent`. Ses boutons « Ouvrir… » et « Enregistrer sous… » font le même travail.

## Le dialogue

La carte et la passerelle échangent des lignes de texte, à 9600 bauds, sur la liaison `Serial` (le câble USB). Les lignes du dialogue commencent par `@` ; les lignes de programme commencent par leur numéro.

| Commande | Échange (→ vers le PC, ← vers la carte) |
|---|---|
| `SAVE` | → `@SAVE NOM.BAS` ← `@OK` → les lignes du programme, puis `@FIN` ← `@OK` |
| `LOAD`, `MERGE` | → `@LOAD NOM.BAS` ← `@OK` (ou `@ERR INTROUVABLE`), puis pour chaque ligne : → `@SUITE` ← la ligne, ou `@FIN` |
| `FILES` | → `@FILES` ← `@OK`, puis pour chaque fichier : → `@SUITE` ← `NOM.EXT taille`, ou `@FIN` |
| L'heure | → `@HEURE` ← `@HEURE JJ/MM/AAAA hh:mm:ss` (heure locale de l'ordinateur) |
| Copie vers le PC (menu 3) | → `@ECRIRE NOM.EXT` ← `@OK` → les octets en hexadécimal, 32 par ligne, puis `@FIN` ← `@OK` |
| Copie depuis le PC (menu 3) | → `@LIRE NOM.EXT` ← `@OK` (ou `@ERR INTROUVABLE`), puis pour chaque ligne : → `@SUITE` ← 32 octets en hexadécimal, ou `@FIN` |

La carte demande les lignes une par une : la mémoire de réception de l'Arduino Mega (64 octets) ne peut pas déborder. La passerelle écrit d'abord un fichier provisoire, puis le renomme : un enregistrement interrompu n'abîme pas l'ancien fichier. Elle ne touche qu'aux fichiers du dossier donné.

On peut écrire sa propre passerelle (dans un autre langage, ou dans un programme qui fait autre chose) en suivant ce dialogue ; il est décrit aussi dans `Basic.ino` (« Fichiers sur le PC »).

## Test

`tester.py` vérifie la passerelle avec le [simulateur](../simulateur/) : c'est le vrai `Basic.ino` qui dialogue avec `passerelle.py`, par une prise à la place du câble USB (Linux ou macOS, pas besoin de pyserial).

```
python3 tester.py
```
