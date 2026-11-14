# rebootMinitel

Redonner vie au Minitel 1B : un croquis et un shield Arduino qui transforment le Minitel en petit ordinateur autonome, avec une horloge et **reBASIC**, un interpréteur BASIC conçu pour l'écran et le clavier du Minitel.

Copyright 2023-2026 — Eric Sérandour — Logiciel libre sous licence [GNU GPLv3](LICENSE).
Site : <https://entropie.org/3615>

---

## Sommaire

- [Matériel](#matériel)
- [Bibliothèques](#bibliothèques)
- [Compilation](#compilation)
- [Utilisation](#utilisation)
- [reBASIC](#rebasic)
  - [Saisie et touches](#saisie-et-touches)
  - [Commandes](#commandes)
  - [Instructions](#instructions)
  - [Attributs d'affichage](#attributs-daffichage)
  - [Fonctions](#fonctions)
  - [Variables et opérateurs](#variables-et-opérateurs)
  - [Dessin](#dessin)
  - [Fichiers de données](#fichiers-de-données)
  - [Broches de l'Arduino](#broches-de-larduino)
  - [Messages d'erreur](#messages-derreur)
  - [Limites](#limites)
  - [Exemples](#exemples)

---

## Matériel

- Minitel 1B (liaison série par la prise péri-informatique DIN)
- Arduino Mega 2560 + **Shield 3615 Mega** (<https://entropie.org/3615/index.php/hardware-2023/>)
- Lecteur de carte microSD (Catalex), sur le bus SPI
- Horloge temps réel **DS3231**, sur le bus I2C
- Buzzer (avec une résistance de 470 Ω)

| Fonction | Broches (Arduino Mega) |
|---|---|
| Minitel | Serial1 : TX1 / RX1 (D18 / D19) |
| Liaison avec le PC | Serial : D0 / D1 |
| Slot USB-A du shield | Serial2 : D16 / D17 |
| Buzzer | D2 |
| Horloge DS3231 | SDA / SCL (D20 / D21), SQW (D34), 32K (D36) |
| Carte SD | MISO D50, MOSI D51, SCK D52, CS D53 |

Le code prévoit aussi l'ESP32 (le Minitel est alors relié à `Serial2`).

## Bibliothèques

- [Minitel1B_Hard](https://github.com/eserandour/Minitel1B_Hard)
- [RTClib](https://github.com/adafruit/RTClib) (Adafruit)
- `SD` et `Wire` (fournies avec l'IDE Arduino)

## Compilation

Compilation réussie avec l'IDE Arduino 1.8.19, carte « Arduino Mega or Mega 2560 ».
Le dossier du croquis doit porter le même nom que le fichier `.ino` principal.

## Utilisation

Au démarrage, le croquis cherche la vitesse du Minitel puis la règle à 4800 bauds. Il vérifie ensuite l'horloge et la carte SD. Un message s'affiche si l'une d'elles est absente, mais le croquis continue sans elle.

Le menu d'accueil, qui affiche la date et l'heure, propose :

| Touche | Menu |
|---|---|
| `0` | Réglage de l'horloge (saisie JJ/MM/AAAA hh:mm, `CORRECTION` pour effacer, `SOMMAIRE` pour revenir sans modifier) |
| `1` | reBASIC |
| `4` | À propos |

---

## reBASIC

Un petit interpréteur BASIC dans l'esprit des BASIC des années 80, adapté au Minitel : 40 colonnes, écran qui défile, touches de fonction, lettres accentuées, couleurs de fond, double taille, caractères semi-graphiques. Les programmes sont enregistrés sur la carte SD.

Pour apprendre à programmer en reBASIC, pas à pas et avec de nombreux exemples : le tutoriel **[Programmer en reBASIC](Tutoriel_reBASIC.pdf)** (aussi en [ODT](Tutoriel_reBASIC.odt)).

```
reBASIC - 2048 octets libres
GUIDE : aide - SOMMAIRE : quitter
Ok
10 FOR I=1 TO 5
20 PRINT "Bonjour Minitel !"; I
30 NEXT
RUN
```

### Saisie et touches

- Une ligne qui commence par un **numéro** (1 à 65535) est rangée dans le programme. Un numéro seul supprime la ligne.
- Sans numéro, la ligne est **exécutée tout de suite** (mode direct).
- Plusieurs instructions peuvent se suivre sur une ligne, séparées par `:`.
- Les minuscules sont converties en majuscules, sauf entre guillemets.
- Une ligne fait au plus 78 caractères.

| Touche | Effet |
|---|---|
| `ENVOI` (ou Retour chariot) | Valide la ligne |
| `CORRECTION` | Efface le dernier caractère |
| `ANNULATION` | Abandonne la ligne en cours / interrompt le programme |
| `GUIDE` | Affiche l'aide (4 pages : `SUITE` et `RETOUR` pour changer de page, `ENVOI` pour revenir à reBASIC) |
| `SOMMAIRE` | Interrompt le programme / quitte reBASIC (le programme en mémoire est conservé) |

### Commandes

| Commande | Rôle |
|---|---|
| `RUN` | Lance le programme (efface d'abord les variables) |
| `EDIT n` | Réaffiche la ligne n dans la zone de saisie, pour la corriger : `CORRECTION` efface la fin, `ENVOI` valide, `ANNULATION` abandonne |
| `LIST [début][-fin]` | Affiche le programme, ou une partie : `LIST 100`, `LIST 100-200`, `LIST -50`, `LIST 300-` |
| `NEW` | Efface le programme et les variables |
| `MEM` | Affiche la mémoire libre pour le programme |
| `RENUM [début][,pas]` | Renumérote le programme (10,10 par défaut) |
| `AIDE` (ou `HELP`) | Affiche l'aide, comme la touche `GUIDE` |
| `SAVE "NOM"` | Enregistre le programme dans `NOM.BAS` sur la carte SD |
| `LOAD "NOM"` | Charge `NOM.BAS` (remplace le programme en cours) |
| `MERGE "NOM"` | Ajoute les lignes de `NOM.BAS` au programme en cours (une ligne de même numéro est remplacée) |
| `FILES` | Liste les fichiers de la carte SD, avec leur taille (les programmes sans leur extension `.BAS`) |
| `KILL "NOM"` | Supprime `NOM.BAS` (ou le fichier nommé, s'il a une extension : `KILL "DONNEES.TXT"`) |

Les noms de fichiers suivent le format 8.3 : 1 à 8 lettres, chiffres, `_` ou `-`, et une extension facultative de 1 à 3 caractères.

`RENUM` met aussi à jour les numéros de ligne écrits après `GOTO`, `GOSUB`, `THEN`, `ELSE`, `RESTORE` et dans les listes de `ON … GOTO/GOSUB`. Les chaînes et les commentaires ne sont pas modifiés. Un numéro calculé (`GOTO 100+X`) ne peut pas être mis à jour. Si la place manque, rien n'est changé.

### Instructions

**Affichage et saisie**

| Instruction | Rôle |
|---|---|
| `PRINT` (ou `?`) | Affiche des nombres et des chaînes. `;` colle les éléments, `,` passe à la colonne suivante (multiple de 10), `TAB(n)` va à la colonne n. Un `;` ou une `,` en fin de ligne évite le retour à la ligne. |
| `INPUT ["texte";] v[,v…]` | Demande une ou plusieurs valeurs au clavier |
| `CLS` | Efface l'écran (et le dessin), et revient à l'affichage normal |
| `LOCATE colonne,rangée` | Place le curseur (colonne 1 à 40, rangée 1 à 24) |

Couleurs, fond, inversion, clignotement, soulignement et taille : voir [Attributs d'affichage](#attributs-daffichage).

**Variables et contrôle**

| Instruction | Rôle |
|---|---|
| `[LET] v = expression` | Affectation (`LET` est facultatif) |
| `IF condition THEN … [ELSE …]` | Après `THEN` et `ELSE` : un numéro de ligne ou des instructions. `IF … GOTO n` est accepté. Chaque `IF` imbriqué sur une même ligne a son propre `ELSE`. |
| `GOTO n` | Saut à la ligne n |
| `GOSUB n` / `RETURN` | Appel de sous-programme / retour (12 niveaux) |
| `ON x GOTO n1,n2…` | Saut à la x-ième ligne de la liste (instruction suivante si x est hors de la liste) |
| `ON x GOSUB n1,n2…` | Pareil, en sous-programme |
| `FOR v = a TO b [STEP c]` / `NEXT [v]` | Boucle (8 niveaux) |
| `WHILE condition` / `WEND` | Boucle tant que la condition est vraie (6 niveaux) |
| `DATA valeur,valeur…` | Données du programme : nombres, chaînes avec ou sans guillemets (sans guillemets, elles passent en majuscules) |
| `READ v[,v…]` | Lit les données suivantes des lignes `DATA` |
| `RESTORE [n]` | Repart du premier `DATA` du programme, ou de la ligne n |
| `DIM A(n)` | Tableau A d'indices 0 à n. Sans `DIM`, un tableau a les indices 0 à 10. |
| `REM …` ou `' …` | Commentaire jusqu'à la fin de la ligne |
| `END` | Fin du programme |
| `STOP` | Arrête le programme |

**Son et attente**

| Instruction | Rôle |
|---|---|
| `BEEP` | Bip du Minitel |
| `SOUND fréquence,durée` | Son sur le buzzer (31 à 20000 Hz, durée en ms) |
| `PAUSE durée` | Attente en millisecondes (`ANNULATION` interrompt aussi le programme pendant l'attente) |

**Dessin** : `PLOT` et `LINE`, voir [Dessin](#dessin).
**Fichiers** : `OPEN`, `CLOSE`, `PRINT #`, `INPUT #`, `LINE INPUT #`, voir [Fichiers de données](#fichiers-de-données).
**Broches** : `PINMODE`, `DWRITE`, `AWRITE`, voir [Broches de l'Arduino](#broches-de-larduino).

### Attributs d'affichage

Les couleurs vont de 0 à 7 : 0 noir, 1 rouge, 2 vert, 3 jaune, 4 bleu, 5 magenta, 6 cyan, 7 blanc. Sur le Minitel 1B, dont l'écran est monochrome, elles apparaissent en niveaux de gris.

| Instruction | Rôle |
|---|---|
| `COLOR n` | Couleur des caractères et des points |
| `PAPER n` | Couleur de fond (voir ci-dessous) |
| `INVERSE n` | Vidéo inverse (1) ou normale (0) |
| `FLASH n` | Clignotement (1) ou texte fixe (0) |
| `UNDERLINE n` | Soulignement (1) ou non (0) |
| `SIZE n` | Taille : 0 normale, 1 double hauteur, 2 double largeur, 3 double taille |

Ces instructions suivent les règles du Minitel (STUM 1B, partie 2, chapitre 2) :

- **`PAPER` et `UNDERLINE` sont des attributs de zone.** En texte, ils ne prennent effet qu'au **premier espace** écrit ensuite (`PAPER 4: PRINT " Titre"`), puis valent pour tout ce qui est écrit sur la même rangée. Sur la rangée suivante, le fond redevient noir jusqu'au premier espace, qui remet la couleur de `PAPER` : revenez à `PAPER 0` quand vous n'en avez plus besoin. En dessin, chaque case semi-graphique prend aussitôt le fond en cours : une case a donc deux couleurs, `COLOR` pour les points et `PAPER` pour le fond.
- **Double hauteur et double taille** : `SIZE 1` ou `SIZE 3` descend le curseur d'une rangée, et le texte occupe la rangée de départ et la suivante. Elles sont sans effet sur la rangée 1.
- **Double largeur** : chaque caractère occupe deux colonnes.
- Le semi-graphique annule la taille, l'inversion et le soulignement du Minitel : reBASIC les rétablit après chaque dessin.
- `CLS`, la fin du programme et les messages d'erreur reviennent à l'affichage normal.

```basic
10 CLS
20 COLOR 0: PAPER 7: PRINT " reBASIC"; TAB(40)
30 COLOR 7: PAPER 0
40 LOCATE 11, 6: SIZE 3: PRINT "LE JEU": SIZE 0
50 LOCATE 11, 10: UNDERLINE 1: PRINT " Hasard et réflexes"
60 UNDERLINE 0
70 LOCATE 10, 20: FLASH 1: PRINT "Appuyez sur une touche"
80 FLASH 0
90 IF INKEY$ = "" THEN 90
```

### Fonctions

**Fonctions numériques**

| Fonction | Résultat |
|---|---|
| `ABS(x)` `INT(x)` `SGN(x)` `SQR(x)` | Valeur absolue, partie entière, signe, racine carrée |
| `SIN(x)` `COS(x)` `TAN(x)` `ATN(x)` | Trigonométrie (angles en radians) |
| `LOG(x)` `EXP(x)` | Logarithme népérien, exponentielle |
| `RND(n)` | Nombre de 0 à 1 si n ≤ 1, entier de 1 à n sinon |
| `LEN(a$)` | Nombre de caractères |
| `VAL(a$)` | Valeur du nombre écrit dans a$ |
| `ASC(a$)` | Code du premier caractère |
| `INSTR([début,]a$,b$)` | Position de b$ dans a$ (0 si absente) |
| `POINT(x,y)` | -1 si le point est allumé, 0 sinon |
| `TIMER` | Millisecondes écoulées depuis le `RUN` |
| `HOUR` `MINUTE` `SECOND` | Heure, minutes, secondes (horloge RTC) |
| `DAY` `MONTH` `YEAR` | Jour, mois, année (horloge RTC) |
| `EOF(n)` | -1 à la fin du fichier n, 0 sinon |
| `DREAD(broche)` | État d'une broche numérique : 0 ou 1 |
| `AREAD(entrée)` | Entrée analogique : 0 à 1023 |

**Fonctions de chaînes**

| Fonction | Résultat |
|---|---|
| `CHR$(n)` | Caractère de code n (1 à 127). Les codes de contrôle sont envoyés tels quels au Minitel : `PRINT CHR$(17);` affiche le curseur, `PRINT CHR$(20);` le cache. |
| `STR$(x)` | Le nombre x écrit en chaîne |
| `HEX$(n)` | Le nombre n en hexadécimal |
| `LEFT$(a$,n)` `RIGHT$(a$,n)` | Les n premiers / derniers caractères |
| `MID$(a$,début[,n])` | n caractères à partir de début (jusqu'à la fin sans n) |
| `UPPER$(a$)` `LOWER$(a$)` | Majuscules / minuscules (lettres sans accent seulement) |
| `INKEY$` | Touche frappée, sans attendre (chaîne vide si aucune) |
| `TIME$` | Heure, « hh:mm:ss » |
| `DATE$` | Date, « JJ/MM/AAAA » |

### Variables et opérateurs

| Type | Noms |
|---|---|
| Nombres (environ 7 chiffres significatifs) | `A` à `Z` |
| Chaînes (32 caractères au plus sur le Mega, 78 sur l'ESP32) | `A$` à `Z$` |
| Tableaux de nombres | `A(n)` à `Z(n)` |

| Opérateurs | |
|---|---|
| Calcul | `+` `-` `*` `/` `^` (touche flèche vers le haut) `MOD` |
| Comparaison | `=` `<>` `<` `>` `<=` `>=` (sur des nombres ou des chaînes) |
| Logique | `AND` `OR` `NOT` (vrai vaut -1, faux vaut 0) |
| Chaînes | `+` pour coller deux chaînes |

### Dessin

L'écran est une grille de **80 × 72 points** (x de 0 à 79, y de 0 à 71, origine en haut à gauche), obtenue avec les caractères semi-graphiques du Minitel : chaque case de texte contient 2 × 3 points.

| Instruction | Rôle |
|---|---|
| `PLOT x,y[,c]` | Allume un point (l'éteint si c = 0) |
| `LINE x1,y1,x2,y2[,c]` | Trace un segment (l'efface si c = 0) |
| `BOX x1,y1,x2,y2[,c[,p]]` | Trace un rectangle (l'efface si c = 0 ; plein si p = 1) |
| `CIRCLE x,y,r[,c[,p]]` | Trace un cercle de centre (x,y) et de rayon r (l'efface si c = 0 ; plein si p = 1) |
| `POINT(x,y)` | -1 si le point est allumé |

- Les points du Minitel sont 1,25 fois plus larges que hauts (matrice de 8 × 10 découpée en 2 × 3). Le rayon de `CIRCLE` est donc mesuré horizontalement, et le cercle s'étend sur r × 1,25 points verticalement, pour paraître rond.
- La couleur des points est celle fixée par `COLOR`, le fond de la case celui fixé par `PAPER`. Une case n'a qu'une couleur de points et une couleur de fond : un nouveau tracé dans une case lui donne les couleurs en cours. Pour peindre une grande surface, effacez un rectangle plein avec le fond voulu : `PAPER 4: BOX 0,0,79,35,0,1`.
- `CLS` efface le dessin.
- Les points sont gardés en mémoire pour pouvoir en allumer plusieurs dans une même case. Cette mémoire ne suit pas le défilement de l'écran : mieux vaut dessiner après un `CLS` et placer le texte avec `LOCATE`.
- La case en bas à droite n'est pas dessinée, car y écrire ferait défiler l'écran.

### Fichiers de données

Des fichiers texte sur la carte SD (extension `.TXT` par défaut), deux ouverts au plus en même temps.

| Instruction | Rôle |
|---|---|
| `OPEN "NOM" FOR INPUT AS #n` | Ouvre un fichier en lecture (n = 1 ou 2) |
| `OPEN "NOM" FOR OUTPUT AS #n` | Crée un fichier (le remplace s'il existe) |
| `OPEN "NOM" FOR APPEND AS #n` | Ouvre un fichier pour écrire à la suite |
| `PRINT #n, …` | Écrit dans le fichier, comme `PRINT` |
| `INPUT #n, v[,v…]` | Lit des valeurs |
| `LINE INPUT #n, A$` | Lit une ligne entière |
| `EOF(n)` | -1 à la fin du fichier |
| `CLOSE [#n[,#m]]` | Ferme un fichier, ou tous sans paramètre |

- Avec `PRINT #`, la virgule écrit une virgule dans le fichier : `PRINT #1, A, B$` écrit par exemple `12,TEXTE`.
- `INPUT #` lit des valeurs séparées par des virgules ou des fins de ligne. Une valeur entre guillemets peut contenir des virgules.
- Les fichiers sont fermés automatiquement par `RUN`, `NEW`, `LOAD`, quand le programme s'arrête (fin, `END`, `STOP`, erreur, interruption) et en quittant reBASIC.

### Broches de l'Arduino

| Instruction / fonction | Rôle |
|---|---|
| `PINMODE broche,mode` | 0 entrée, 1 sortie, 2 entrée avec résistance de tirage interne (bouton vers GND : appuyé = 0) |
| `DWRITE broche,valeur` | Sortie numérique : 0 ou 1 |
| `AWRITE broche,valeur` | Sortie MLI (PWM) : 0 à 255 |
| `DREAD(broche)` | Lecture numérique : 0 ou 1 |
| `AREAD(entrée)` | Lecture analogique : 0 à 1023 (entrée A0 à A15 sur le Mega, numéro de broche sur l'ESP32) |

Le mode 2 (`INPUT_PULLUP` de l'Arduino) relie la broche au +5 V par une résistance interne. Un bouton se branche alors simplement entre la broche et GND : `DREAD` renvoie 1 quand il est relâché et 0 quand il est appuyé (logique inversée). En mode 0, une entrée reliée à rien donne un état imprévisible.

```basic
10 PINMODE 7,2
20 IF DREAD(7)=0 THEN PRINT "Appuyé" ELSE PRINT "Relâché"
30 PAUSE 200: GOTO 20
```

Les broches utilisées par le Minitel, la carte SD, l'horloge, le buzzer et les liaisons série sont refusées (erreur « Broche interdite »).

### Messages d'erreur

En cas d'erreur, reBASIC donne le message et le numéro de la ligne, puis réaffiche la ligne avec, **en vidéo inverse, l'endroit où l'analyse s'est arrêtée** :

```
Erreur de syntaxe en ligne 10
10 PRINT 1+*2          (le * apparaît en vidéo inverse)
```

| Message | Cause |
|---|---|
| Erreur de syntaxe | Instruction mal écrite |
| Division par zéro | |
| Ligne n inexistante | `GOTO`, `GOSUB`, `RESTORE`… vers une ligne qui n'existe pas |
| RETURN sans GOSUB / NEXT sans FOR / WEND sans WHILE / WHILE sans WEND | Structures mal appariées |
| Mémoire pleine | Programme ou tableaux trop grands |
| Ligne trop longue | `EDIT` d'une ligne de plus de 78 caractères (possible après un `RENUM`) |
| Type incompatible | Nombre à la place d'une chaîne, ou l'inverse |
| Indice hors limites | Indice de tableau en dehors de 0 à n |
| Trop d'imbrications | Trop de parenthèses, `GOSUB`, `FOR` ou `WHILE` imbriqués |
| Valeur incorrecte | Paramètre hors des limites permises |
| Plus de DATA | `READ` après la dernière donnée |
| Fichier introuvable / Fichier déjà ouvert / Fichier non ouvert / Fin de fichier / Erreur de fichier | Fichiers sur la carte SD |
| Broche interdite | Broche réservée au matériel |
| Horloge absente | Fonction d'heure ou de date sans horloge RTC |
| Interruption | `ANNULATION` ou `SOMMAIRE` pendant l'exécution |

### Limites

| | Arduino Mega | ESP32 |
|---|---|---|
| Taille du programme | 2 048 octets | 16 384 octets |
| Longueur des chaînes | 32 caractères | 78 caractères |
| Éléments de tableaux (au total) | 64 | 1 000 |
| Parenthèses et fonctions imbriquées | 6 niveaux | 8 niveaux |

Ces tailles se règlent au début de `Basic.ino` (`BAS_PROG_SIZE`, `BAS_STR_MAX`, `BAS_ARR_SIZE`…), selon la mémoire disponible.

- `TIMER` perd de la précision après environ 4 h 30 (nombres sur 7 chiffres significatifs).
- Une boucle `WHILE` sans `WEND` n'est signalée que si sa condition est fausse.

### Exemples

**Une horloge**

```basic
10 CLS
20 LOCATE 15,12: PRINT TIME$;
30 PAUSE 500: GOTO 20
```

**Lire des données**

```basic
10 FOR I=1 TO 3
20 READ N$, A
30 PRINT N$; " a "; A; " ans"
40 NEXT
50 DATA "Alice",12,"Bruno",9,"Chloé",11
```

**Un menu avec ON … GOSUB**

```basic
10 INPUT "Choix (1-3)"; C
20 ON C GOSUB 100,200,300
30 GOTO 10
100 PRINT "Un": RETURN
200 PRINT "Deux": RETURN
300 PRINT "Trois": RETURN
```

**Un journal sur la carte SD**

```basic
10 OPEN "JOURNAL" FOR APPEND AS #1
20 PRINT #1, DATE$, TIME$, AREAD(0)
30 CLOSE #1
40 OPEN "JOURNAL" FOR INPUT AS #1
50 WHILE NOT EOF(1)
60 LINE INPUT #1, L$: PRINT L$
70 WEND
80 CLOSE
```

**Un dessin**

```basic
10 CLS
20 PAPER 4: BOX 0, 0, 79, 44, 0, 1
30 PAPER 2: BOX 0, 45, 79, 71, 0, 1
40 COLOR 3: PAPER 4: CIRCLE 62, 14, 7, 1, 1
50 COLOR 7: PAPER 2: BOX 12, 33, 35, 62, 1, 1
60 COLOR 1: BOX 20, 45, 27, 62, 1, 1
70 GOTO 70
```

---

## Outils

Le dossier [`outils/`](outils/) contient, pour l'ordinateur :

- un **simulateur** de reBASIC, qui compile le vrai `Basic.ino` sur PC : exécuter un programme, utiliser reBASIC dans un terminal, produire des images d'écran ;
- la **fabrication du tutoriel** « Programmer en reBASIC » (ODT et PDF), dont tous les exemples sont exécutés dans le simulateur.
