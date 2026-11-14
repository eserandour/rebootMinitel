---
title: "Programmer en reBASIC"
subtitle: "Tutoriel pour le Minitel 1B et rebootMinitel"
date: "Novembre 2026"
lang: fr-FR
---

# Avant de commencer

## Ce que vous allez apprendre

**reBASIC** est un langage BASIC installé dans **rebootMinitel**, le croquis Arduino qui transforme un Minitel 1B en petit ordinateur autonome. Le BASIC est le langage avec lequel des millions de personnes ont appris à programmer dans les années 1980 : ses instructions sont des mots anglais simples (`PRINT` pour afficher, `INPUT` pour demander, `IF` pour « si »…), et l'on voit tout de suite le résultat de ce que l'on écrit.

Ce tutoriel part de zéro. Chaque chapitre introduit quelques notions, les illustre par des exemples courts que vous pouvez taper tels quels, puis propose des exercices. Les corrigés sont à la fin du document.

Toutes les images d'écran de ce document ont été produites en exécutant réellement les exemples dans reBASIC : ce que vous voyez est ce que votre Minitel affichera.

## Ce qu'il vous faut

- un Minitel 1B relié à la carte rebootMinitel (Arduino Mega et Shield 3615 Mega) ;
- une carte microSD dans le lecteur, pour enregistrer vos programmes (chapitre 14) ;
- l'horloge DS3231, pour les fonctions de date et d'heure (chapitre 13).

## Comment lire les exemples

Les programmes à taper sont présentés ainsi :

```basic
10 PRINT "Bonjour"
```

Et ce que le Minitel affiche apparaît dans une image d'écran, comme celle-ci :

```basic
10 PRINT "Bonjour"
```
<!--ecran-->

::: {custom-style="Astuce"}
**Astuce** — Les encadrés comme celui-ci donnent des conseils pratiques.
:::

::: {custom-style="Attention"}
**Attention** — Ceux-ci signalent les pièges les plus courants.
:::

# Premiers pas

## Entrer dans reBASIC

Allumez le Minitel et la carte rebootMinitel. Sur l'écran d'accueil, appuyez sur la touche **1**. reBASIC vous accueille ainsi :

<!--ecran mode=accueil-->

- « 2048 octets libres » : la place disponible pour votre programme.
- **GUIDE** affiche l'aide (trois pages que l'on parcourt avec **SUITE** et **RETOUR** ; **ENVOI** ramène à reBASIC).
- **SOMMAIRE** quitte reBASIC et revient au menu. Votre programme n'est pas perdu : il est toujours là quand vous revenez.
- **Ok** signifie que reBASIC attend vos ordres.

## Le Minitel comme calculatrice

Tapez la ligne suivante, puis appuyez sur **ENVOI** :

```basic
PRINT 2+3
```
<!--ecran mode=direct-->

reBASIC a exécuté l'ordre immédiatement : c'est le **mode direct**. `PRINT` signifie « affiche ». Essayez d'autres calculs :

```basic
PRINT 7*6
PRINT 10/4
PRINT (2+3)*4
PRINT 2^10
? 100-1
```
<!--ecran mode=direct-->

- `*` est la multiplication, `/` la division, `^` la puissance (sur le clavier du Minitel, c'est la touche **↑**).
- Les parenthèses fixent l'ordre des calculs, comme en mathématiques.
- Le point d'interrogation `?` est un raccourci pour `PRINT`.
- Les nombres décimaux s'écrivent avec un **point** : `3.5` et non `3,5`.

::: {custom-style="Astuce"}
**Astuce** — Vous pouvez taper en minuscules : `print 2+3` fonctionne aussi. reBASIC convertit tout en majuscules, sauf le texte entre guillemets.
:::

## Afficher du texte

Le texte à afficher se met entre guillemets :

```basic
PRINT "Bonjour Minitel !"
PRINT "2+3"
PRINT 2+3
```
<!--ecran mode=direct-->

Entre guillemets, `2+3` n'est pas un calcul mais trois caractères. Les lettres accentuées minuscules (é, è, à, ç, ê…) s'affichent normalement ; le Minitel n'a pas de majuscules accentuées.

## Corriger une erreur de frappe

- Pendant la frappe, la touche **CORRECTION** efface le dernier caractère.
- La touche **ANNULATION** abandonne toute la ligne.

Si reBASIC ne comprend pas une ligne, il le dit, et montre en **vidéo inverse** l'endroit où il s'est arrêté :

```basic
PRINT 2+*3
```
<!--ecran mode=direct erreur-->

Ici, il ne s'attendait pas à trouver `*` juste après `+`. Le chapitre 15 explique tous les messages d'erreur.

# Votre premier programme

## Des lignes numérotées

En mode direct, l'ordre est exécuté puis oublié. Pour que reBASIC **retienne** des ordres, on les fait précéder d'un **numéro de ligne** :

```basic
10 PRINT "Je suis un Minitel."
20 PRINT "J'ai plus de 40 ans."
30 PRINT "Et je programme en BASIC !"
```

Quand vous tapez ces lignes, rien ne se passe : elles sont rangées dans la mémoire. L'ensemble de ces lignes forme un **programme**. Pour l'exécuter, tapez `RUN` :

```basic
10 PRINT "Je suis un Minitel."
20 PRINT "J'ai plus de 40 ans."
30 PRINT "Et je programme en BASIC !"
```
<!--ecran-->

Les lignes sont exécutées dans l'ordre des numéros, quel que soit l'ordre dans lequel vous les avez tapées.

## Voir, modifier et effacer des lignes

- `LIST` affiche le programme. `LIST 20` affiche la ligne 20, `LIST 10-20` les lignes 10 à 20.
- Pour **modifier** une ligne, retapez-la entièrement avec le même numéro : la nouvelle remplace l'ancienne.
- Pour **insérer** une ligne, choisissez un numéro intermédiaire : `15` se place entre `10` et `20`.
- Pour **supprimer** une ligne, tapez seulement son numéro.
- `NEW` efface tout le programme.

```basic
10 PRINT "Je suis un Minitel."
20 PRINT "J'ai plus de 40 ans."
30 PRINT "Et je programme en BASIC !"
LIST
20 PRINT "J'ai 44 ans."
15 PRINT "---"
30
LIST
```
<!--ecran mode=direct-->

::: {custom-style="Astuce"}
**Astuce** — Numérotez de 10 en 10 : il restera de la place pour insérer des lignes. Et si la place manque, `RENUM` renumérote tout le programme de 10 en 10 (chapitre 14).
:::

## Plusieurs instructions sur une ligne

Le signe deux-points `:` sépare plusieurs instructions sur une même ligne :

```basic
10 PRINT "Un": PRINT "Deux": PRINT "Trois"
```
<!--ecran-->

Une ligne fait au plus **78 caractères**, numéro compris.

## Commenter son programme

`REM` (pour *remark*, remarque) ou l'apostrophe `'` introduisent un **commentaire** : tout ce qui suit sur la ligne est ignoré par reBASIC. Les commentaires servent à expliquer le programme… à vous-même, quelques semaines plus tard. Après une instruction, un commentaire se place derrière un deux-points, comme une nouvelle instruction.

```basic
10 REM Mon premier programme
20 PRINT "Bonjour": ' on salue
```
<!--ecran-->

## Effacer l'écran

`CLS` (*clear screen*) efface l'écran et place le curseur en haut à gauche. La plupart des programmes commencent par là.

# Les variables

## Ranger des nombres

Une **variable** est une case de la mémoire qui porte un nom et contient une valeur. En reBASIC, les variables numériques s'appellent par **une seule lettre**, de `A` à `Z` :

```basic
10 A = 5
20 B = 3
30 PRINT A + B
40 A = A * 10
50 PRINT A
```
<!--ecran-->

- `A = 5` se lit « A **reçoit** 5 ». On peut écrire `LET A = 5`, à l'ancienne, mais `LET` est facultatif.
- `A = A * 10` calcule `A * 10` avec l'ancienne valeur de `A` (5), puis range le résultat (50) dans `A`.
- Une variable qui n'a encore rien reçu vaut 0.

::: {custom-style="Attention"}
**Attention** — Les noms de variables n'ont qu'**une lettre**. `SCORE = 10` ou `AB = 3` provoquent une erreur de syntaxe. Choisissez des lettres parlantes : `S` pour le score, `V` pour les vies, `N` pour un nombre…
:::

## Ranger du texte

Une variable de **chaîne** (de caractères) porte un nom suivi d'un dollar : `A$` à `Z$`.

```basic
10 N$ = "Marianne"
20 V$ = "Paris"
30 PRINT N$; " habite à "; V$
```
<!--ecran-->

`A`, `A$` et le tableau `A()` (chapitre 9) sont trois variables différentes.

::: {custom-style="Attention"}
**Attention** — Sur la carte Arduino Mega, une chaîne contient au plus **32 caractères** (une lettre accentuée compte pour deux). Au-delà, la fin est perdue. C'est vrai aussi pour un texte entre guillemets : coupez les longs textes en plusieurs `PRINT`.
:::

## Coller des chaînes

Le signe `+` colle deux chaînes bout à bout :

```basic
10 P$ = "Bon"
20 S$ = "jour"
30 M$ = P$ + S$ + " !"
40 PRINT M$
```
<!--ecran-->

## Garder les variables après le programme

Quand un programme est terminé, ses variables gardent leur valeur : on peut les afficher en mode direct pour comprendre ce qui s'est passé. `RUN` remet toutes les variables à zéro avant de lancer le programme.

```basic
10 A = 6: B = 7: C = A * B
RUN
PRINT C
```
<!--ecran mode=direct-->

# Afficher avec style

## Les séparateurs de PRINT

Dans un `PRINT`, on peut afficher plusieurs éléments à la suite :

- le **point-virgule** `;` les colle l'un à l'autre ;
- la **virgule** `,` passe à la colonne suivante, de 10 en 10 (colonnes 1, 11, 21, 31) : pratique pour faire des tableaux ;
- un `;` à la **fin** de la ligne empêche le retour à la ligne : le `PRINT` suivant continuera sur la même ligne.

```basic
10 A = 12
20 PRINT "A vaut "; A; " ."
30 PRINT "Nom", "Age", "Ville"
40 PRINT "Léa", 12, "Lille"
50 PRINT "Sur la ";
60 PRINT "même ligne"
```
<!--ecran-->

::: {custom-style="Astuce"}
**Astuce** — reBASIC n'ajoute pas d'espace autour des nombres : pensez à en mettre dans vos textes, comme `"A vaut "`.
:::

## Choisir la colonne : TAB

`TAB(n)` avance jusqu'à la colonne `n` (de 1 à 40) :

```basic
10 PRINT "Produit"; TAB(20); "Prix"
20 PRINT "Croissant"; TAB(20); 1.2
30 PRINT "Baguette"; TAB(20); 1.1
```
<!--ecran-->

## Placer le texte où l'on veut : LOCATE

L'écran du Minitel a **40 colonnes** et **24 rangées**. `LOCATE colonne, rangée` place le curseur ; le `PRINT` suivant écrit à cet endroit.

```basic
10 CLS
20 LOCATE 1,1: PRINT "En haut à gauche"
30 LOCATE 15,12: PRINT "Au milieu"
40 LOCATE 25,24: PRINT "En bas à droite";
50 GOTO 50
```
<!--ecran rows=24-->

Le `;` à la fin de la ligne 40 évite le passage à la ligne, qui ferait défiler l'écran depuis la rangée 24. La ligne 50 fait tourner le programme sur place (`GOTO` est expliqué au chapitre 6) : sans elle, le programme se terminerait et le `Ok` final ferait défiler l'écran. Appuyez sur **ANNULATION** pour arrêter.

::: {custom-style="Attention"}
**Attention** — Quand on écrit sur la dernière rangée, en arrivant au bout de la ligne, ou avec un retour à la ligne, l'écran **défile** vers le haut. Sur la rangée 24, terminez vos `PRINT` par un `;` et n'écrivez pas dans la colonne 40.
:::

## Couleurs et vidéo inverse

`COLOR n` choisit la couleur des caractères qui suivent, de 0 à 7 : 0 noir, 1 rouge, 2 vert, 3 jaune, 4 bleu, 5 magenta, 6 cyan, 7 blanc. Sur le Minitel 1B, dont l'écran est monochrome, les couleurs apparaissent en **niveaux de gris**. `INVERSE 1` passe en **vidéo inverse** (caractères sombres sur fond clair) et `INVERSE 0` revient à l'affichage normal.

```basic
10 CLS
20 FOR C = 1 TO 7
30 COLOR C: PRINT "Couleur numéro "; C
40 NEXT
50 COLOR 7: INVERSE 1: PRINT " MENU "
60 INVERSE 0: PRINT "Retour à la normale"
```
<!--ecran-->

À la fin d'un programme, reBASIC revient tout seul à l'affichage normal (blanc, sans inverse).

# Dialoguer avec l'utilisateur

## INPUT : poser une question

`INPUT` attend que l'utilisateur tape une valeur et la range dans une variable. Le Minitel affiche un point d'interrogation et le curseur.

```basic
10 INPUT "Votre prénom"; P$
20 INPUT "Votre âge"; A
30 PRINT "Bonjour "; P$; " !"
40 PRINT "Dans 10 ans, vous aurez "; A + 10; " ans."
```
<!--ecran keys="Camille\n33\n"-->

- Le texte entre guillemets est facultatif : `INPUT A` affiche seulement `?`.
- Si la variable est numérique et que l'on tape autre chose qu'un nombre, reBASIC affiche « Nombre attendu » et repose la question.
- On peut demander plusieurs valeurs d'un coup : `INPUT "Largeur, hauteur"; L, H` pose une question par variable.

## Convertir entre nombres et chaînes

- `VAL(A$)` donne la valeur du nombre écrit dans la chaîne : `VAL("12.5")` vaut 12.5.
- `STR$(N)` donne la chaîne qui représente le nombre : `STR$(12.5)` vaut `"12.5"`.

```basic
10 A$ = "40"
20 PRINT A$ + "2"
30 PRINT VAL(A$) + 2
40 PRINT "Total : " + STR$(VAL(A$) * 3)
```
<!--ecran-->

## Un petit convertisseur

```basic
10 CLS
20 PRINT "Conversion francs -> euros"
30 INPUT "Montant en francs"; F
40 E = INT(F / 6.55957 * 100 + 0.5) / 100
50 PRINT F; " F = "; E; " euros"
```
<!--ecran keys="100\n"-->

La ligne 40 arrondit au centime : `INT` garde la partie entière (chapitre 10).

## Exercices

1. Écrivez un programme qui demande deux nombres et affiche leur somme, leur différence, leur produit et leur quotient.
2. Écrivez un programme qui demande le rayon d'un cercle et affiche son périmètre (2 × π × R) et sa surface (π × R²). Prenez π = 3.14159.

# Prendre des décisions

## IF … THEN

`IF condition THEN action` exécute l'action seulement si la condition est vraie.

```basic
10 INPUT "Quel âge as-tu"; A
20 IF A >= 18 THEN PRINT "Tu es majeur."
30 IF A < 18 THEN PRINT "Tu es mineur."
```
<!--ecran keys="15\n"-->

Les comparaisons possibles :

| Signe | Signification |
|:---:|---|
| `=` | égal |
| `<>` | différent |
| `<` | strictement inférieur |
| `>` | strictement supérieur |
| `<=` | inférieur ou égal |
| `>=` | supérieur ou égal |

## ELSE : sinon

`ELSE` donne ce qu'il faut faire quand la condition est fausse. Tout doit tenir sur la même ligne.

```basic
10 INPUT "Un nombre"; N
20 IF N MOD 2 = 0 THEN PRINT "Pair" ELSE PRINT "Impair"
```
<!--ecran keys="7\n"-->

`N MOD 2` est le reste de la division de N par 2 : 0 pour un nombre pair.

Après `THEN` et `ELSE`, on peut mettre **plusieurs instructions** séparées par `:` : elles font toutes partie de la branche.

```basic
10 INPUT "Mot de passe"; M$
20 IF M$ = "minitel" THEN PRINT "Bienvenue": B = 1 ELSE PRINT "Refusé": B = 0
30 PRINT "B vaut "; B
```
<!--ecran keys="minitel\n"-->

## Combiner des conditions : AND, OR, NOT

- `AND` (et) : les deux conditions doivent être vraies.
- `OR` (ou) : il suffit qu'une des deux soit vraie.
- `NOT` (non) : inverse une condition.

```basic
10 INPUT "Température"; T
20 IF T >= 18 AND T <= 24 THEN PRINT "Agréable"
30 IF T < 5 OR T > 35 THEN PRINT "Extrême !"
40 IF NOT (T > 0) THEN PRINT "Il gèle."
```
<!--ecran keys="-3\n"-->

::: {custom-style="Astuce"}
**Pour les curieux** — Pour reBASIC, une condition vraie vaut **-1** et une condition fausse vaut **0** : essayez `PRINT 3 > 2` en mode direct. Et toute valeur différente de 0 est considérée comme vraie : `IF A THEN …` signifie « si A n'est pas nul ».
:::

## Comparer des chaînes

Les chaînes se comparent aussi. L'ordre est celui des codes des caractères : les majuscules viennent avant les minuscules, et `"Z"` est avant `"a"`.

```basic
10 INPUT "Ville"; V$
20 IF V$ = "Paris" THEN PRINT "La capitale !" ELSE PRINT "Belle ville."
30 IF V$ < "M" THEN PRINT "Entre A et L dans l'annuaire."
```
<!--ecran keys="Lyon\n"-->

## GOTO : sauter à une ligne

`GOTO n` poursuit l'exécution à la ligne `n`. Après `THEN` ou `ELSE`, un numéro seul est un `GOTO` abrégé.

```basic
10 INPUT "Combien font 6 x 7"; R
20 IF R = 42 THEN 50
30 PRINT "Non, essaie encore."
40 GOTO 10
50 PRINT "Bravo !"
```
<!--ecran keys="48\n42\n"-->

## Le jeu du nombre mystère

Voici un premier vrai jeu. `RND(100)` tire un nombre entier au hasard entre 1 et 100 (chapitre 10).

```basic
10 CLS: PRINT "J'ai choisi un nombre"
15 PRINT "entre 1 et 100."
20 M = RND(100): C = 0
30 INPUT "Ta proposition"; P
40 C = C + 1
50 IF P < M THEN PRINT "C'est plus.": GOTO 30
60 IF P > M THEN PRINT "C'est moins.": GOTO 30
70 PRINT "Gagné en "; C; " coups !"
```
<!--ecran keys="50\n75\n88\n81\n85\n83\n84\n"-->

## Exercices

3. Demandez une note sur 20 et affichez « Très bien » (16 ou plus), « Bien » (de 12 à 15), « Passable » (de 10 à 11) ou « Insuffisant ».
4. Demandez une année et dites si elle est bissextile : divisible par 4 mais pas par 100, ou divisible par 400.

# Répéter

## La boucle FOR … NEXT

`FOR` répète un bloc d'instructions en faisant varier une variable :

```basic
10 FOR I = 1 TO 5
20 PRINT "Tour numéro "; I
30 NEXT I
```
<!--ecran-->

`FOR I = 1 TO 5` donne à `I` la valeur 1 ; à chaque `NEXT`, `I` augmente de 1 et la boucle recommence, tant que `I` ne dépasse pas 5. Le nom de la variable après `NEXT` est facultatif.

## Le pas : STEP

`STEP` change le pas, qui peut être décimal ou négatif :

```basic
10 FOR I = 10 TO 0 STEP -2: PRINT I; " ";: NEXT
20 PRINT
30 FOR X = 0 TO 1 STEP 0.25: PRINT X; " ";: NEXT
40 PRINT: PRINT "Décollage !"
```
<!--ecran-->

Un `PRINT` seul passe simplement à la ligne.

## Une table de multiplication

```basic
10 INPUT "Quelle table"; T
20 FOR I = 1 TO 10
30 PRINT T; " x "; I; " = "; T * I
40 NEXT
```
<!--ecran keys="7\n"-->

## Des boucles imbriquées

Une boucle peut en contenir une autre. Ici, la boucle sur `C` (les colonnes) est entièrement parcourue pour chaque valeur de `L` (les lignes) :

```basic
10 FOR L = 1 TO 5
20 FOR C = 1 TO L
30 PRINT "*";
40 NEXT C
50 PRINT
60 NEXT L
```
<!--ecran-->

## La boucle WHILE … WEND

`WHILE condition` répète les instructions jusqu'à `WEND` **tant que** la condition est vraie. On l'utilise quand on ne sait pas d'avance combien de tours il faudra.

```basic
10 S = 0: N = 0
20 WHILE S <= 100
30 N = N + 1: S = S + N
40 WEND
50 PRINT "1+2+...+"; N; " = "; S; " dépasse 100"
```
<!--ecran-->

Si la condition est fausse dès le départ, les instructions de la boucle ne sont jamais exécutées.

## Boucler à l'infini et s'arrêter

Un `GOTO` vers une ligne précédente crée une boucle sans fin. Pour arrêter un programme, quel qu'il soit, appuyez sur **ANNULATION** (ou **SOMMAIRE**) : reBASIC indique où il s'est arrêté.

```basic
10 N = 0
20 N = N + 1
30 PRINT N;
40 GOTO 20
```
<!--ecran keys="{WAIT:1500}{ANNUL}" last=8 erreur-->

## Attendre : PAUSE

`PAUSE n` attend `n` millisecondes (1000 ms = 1 seconde).

```basic
10 FOR I = 10 TO 0 STEP -1
20 PRINT I; "... ";
30 PAUSE 1000
40 NEXT
50 PRINT: PRINT "Partez !"
```
<!--ecran-->

## Exercices

5. Affichez les carrés des nombres de 1 à 12, sous la forme `5 x 5 = 25`.
6. Un capital de 1000 euros est placé à 3 % par an. Avec `WHILE`, calculez au bout de combien d'années il aura doublé.
7. Dessinez un triangle d'étoiles « à l'envers » : 5 étoiles, puis 4, puis 3…

# Organiser son programme

## Les sous-programmes : GOSUB et RETURN

Quand une même suite d'instructions sert à plusieurs endroits, on l'écrit une seule fois, dans un **sous-programme**. `GOSUB n` va à la ligne `n` ; `RETURN` revient juste après le `GOSUB`.

```basic
10 CLS
20 T$ = "Bienvenue": GOSUB 100
30 PRINT "Ceci est un programme."
40 T$ = "Au revoir": GOSUB 100
50 END
100 REM Sous-programme : titre encadré
110 PRINT "+------------------+"
120 PRINT "| "; T$; TAB(20); "|"
130 PRINT "+------------------+"
140 RETURN
```
<!--ecran-->

`END` termine le programme. Sans lui, reBASIC continuerait à la ligne 100 et tomberait sur un `RETURN` sans `GOSUB`.

::: {custom-style="Astuce"}
**Astuce** — Placez les sous-programmes à la fin, à partir d'un numéro rond (100, 1000…), et commencez chacun par un `REM` qui dit ce qu'il fait.
:::

## Choisir parmi plusieurs : ON … GOTO / ON … GOSUB

`ON x GOSUB n1, n2, n3` appelle le sous-programme `n1` si `x` vaut 1, `n2` s'il vaut 2, etc. Si `x` est en dehors de la liste, reBASIC passe à l'instruction suivante. C'est idéal pour un menu :

```basic
10 CLS: PRINT "1 - Bonjour"
20 PRINT "2 - Heure"
30 PRINT "3 - Quitter"
40 INPUT "Choix"; C
50 ON C GOSUB 100, 200, 300
60 IF C <> 3 THEN 40
70 END
100 PRINT "Bonjour à vous !": RETURN
200 PRINT "Il est "; TIME$: RETURN
300 PRINT "A bientôt.": RETURN
```
<!--ecran keys="1\n2\n5\n3\n"-->

`ON x GOTO n1, n2…` fonctionne de la même façon, avec des sauts au lieu d'appels.

# Les tableaux

## Une variable, plusieurs cases

Un **tableau** est une variable qui contient plusieurs valeurs numérotées. `DIM N(5)` crée le tableau `N` avec les cases `N(0)` à `N(5)`. Le numéro entre parenthèses, l'**indice**, peut être une expression.

```basic
10 DIM N(5)
20 FOR I = 1 TO 5
30 INPUT "Note"; N(I)
40 NEXT
50 S = 0
60 FOR I = 1 TO 5: S = S + N(I): NEXT
70 PRINT "Moyenne : "; S / 5
80 M = N(1)
90 FOR I = 2 TO 5: IF N(I) > M THEN M = N(I)
100 NEXT
110 PRINT "Meilleure note : "; M
```
<!--ecran keys="12\n15\n9\n17\n13\n"-->

::: {custom-style="Attention"}
**Attention** — Ligne 90 : la condition fausse d'un `IF` fait sauter **toute la fin de la ligne**, `NEXT` compris. C'est pourquoi `NEXT` est seul sur la ligne 100. Retenez la règle : un `IF` est toujours la dernière instruction utile de sa ligne.
:::

- Sans `DIM`, un tableau a automatiquement les indices 0 à 10.
- Un indice en dehors des limites provoque l'erreur « Indice hors limites ».
- Il n'existe que des tableaux de **nombres**, à une dimension.
- Sur l'Arduino Mega, tous les tableaux réunis ont au plus **64 cases** (1 000 sur ESP32).

## Compter avec un tableau : lancers de dé

```basic
10 DIM F(6)
20 FOR I = 1 TO 600
30 D = RND(6): F(D) = F(D) + 1
40 NEXT
50 FOR D = 1 TO 6
60 PRINT D; " : "; F(D); " fois"
70 NEXT
```
<!--ecran norand-->

Chaque face sort environ 100 fois sur 600 : c'est le hasard qui s'équilibre.

## Exercice

8. Remplissez un tableau de 10 nombres tirés au hasard entre 1 et 50, affichez-les, puis affichez-les à nouveau **triés** du plus petit au plus grand. Indice : parcourez le tableau et échangez deux voisins s'ils sont dans le mauvais ordre ; recommencez tant qu'il y a eu un échange.

# Travailler avec du texte

## Longueur et morceaux de chaînes

| Fonction | Résultat |
|---|---|
| `LEN(A$)` | nombre de caractères |
| `LEFT$(A$, n)` | les `n` premiers caractères |
| `RIGHT$(A$, n)` | les `n` derniers caractères |
| `MID$(A$, d, n)` | `n` caractères à partir du `d`-ième (jusqu'à la fin sans `n`) |
| `INSTR(A$, B$)` | position de `B$` dans `A$`, 0 si absente |
| `UPPER$(A$)`, `LOWER$(A$)` | majuscules, minuscules (lettres sans accent) |

```basic
10 A$ = "rebootMinitel"
20 PRINT LEN(A$)
30 PRINT LEFT$(A$, 6)
40 PRINT RIGHT$(A$, 7)
50 PRINT MID$(A$, 7, 4)
60 PRINT INSTR(A$, "Mini")
70 PRINT UPPER$(A$)
```
<!--ecran-->

`INSTR(d, A$, B$)` commence la recherche au `d`-ième caractère : on peut ainsi trouver toutes les occurrences.

## Parcourir une chaîne caractère par caractère

```basic
10 INPUT "Un mot"; M$
20 R$ = ""
30 FOR I = LEN(M$) TO 1 STEP -1
40 R$ = R$ + MID$(M$, I, 1)
50 NEXT
60 PRINT "A l'envers : "; R$
70 IF R$ = M$ THEN PRINT "C'est un palindrome !"
```
<!--ecran keys="radar\n"-->

## Codes des caractères : ASC et CHR$

Chaque caractère a un **code** numérique. `ASC(A$)` donne le code du premier caractère, `CHR$(n)` le caractère de code `n` (de 1 à 127).

```basic
10 PRINT ASC("A"); " "; ASC("a"); " "; ASC("0")
20 FOR I = 65 TO 90: PRINT CHR$(I);: NEXT
30 PRINT
40 PRINT HEX$(255); " "; HEX$(4096)
```
<!--ecran-->

`HEX$(n)` écrit un nombre en hexadécimal (base 16).

::: {custom-style="Astuce"}
**Pour les curieux** — Les codes inférieurs à 32 sont des **codes de commande** que reBASIC envoie tels quels au Minitel. Par exemple `PRINT CHR$(7);` fait biper le Minitel, `PRINT CHR$(17);` affiche le curseur et `PRINT CHR$(20);` le cache.
:::

## Le code de César

Chaque lettre est remplacée par celle qui se trouve 3 rangs plus loin dans l'alphabet :

```basic
10 INPUT "Message"; M$
20 M$ = UPPER$(M$): C$ = ""
30 FOR I = 1 TO LEN(M$)
40 K = ASC(MID$(M$, I, 1))
50 IF K >= 65 AND K <= 90 THEN K = (K - 65 + 3) MOD 26 + 65
60 C$ = C$ + CHR$(K)
70 NEXT
80 PRINT "Codé : "; C$
```
<!--ecran keys="Rendez-vous a midi\n"-->

## Exercice

9. Demandez une phrase et comptez combien elle contient de voyelles (a, e, i, o, u, y). Astuce : `INSTR("AEIOUY", L$)` est différent de 0 si la lettre `L$` est une voyelle majuscule.

# Calculs et hasard

## Les fonctions mathématiques

| Fonction | Résultat |
|---|---|
| `ABS(x)` | valeur absolue (`ABS(-3)` = 3) |
| `INT(x)` | partie entière, arrondie vers le bas (`INT(2.7)` = 2, `INT(-2.7)` = -3) |
| `SGN(x)` | signe : -1, 0 ou 1 |
| `SQR(x)` | racine carrée |
| `SIN(x)`, `COS(x)`, `TAN(x)`, `ATN(x)` | trigonométrie, angles en **radians** |
| `LOG(x)`, `EXP(x)` | logarithme népérien, exponentielle |
| `a MOD b` | reste de la division entière de `a` par `b` |

```basic
10 PRINT SQR(2)
20 PRINT INT(7 / 2); " reste "; 7 MOD 2
30 P = 4 * ATN(1): PRINT "Pi = "; P
40 PRINT SIN(P / 6)
50 PRINT EXP(1)
60 PRINT 2 ^ 0.5
```
<!--ecran-->

Les nombres ont environ **7 chiffres significatifs**. Les très grands et très petits s'écrivent en notation scientifique : `1.5E9` signifie 1,5 × 10⁹.

## Arrondir

`INT(x + 0.5)` arrondit à l'unité la plus proche ; pour arrondir à 2 décimales, on multiplie par 100 avant et on divise par 100 après :

```basic
10 X = 3.14159
20 PRINT INT(X + 0.5)
30 PRINT INT(X * 100 + 0.5) / 100
```
<!--ecran-->

## Le hasard : RND

- `RND(n)` avec `n` > 1 donne un **entier** au hasard de 1 à `n` : `RND(6)` simule un dé.
- `RND(1)` donne un nombre décimal entre 0 et 1 (1 exclu).

```basic
10 FOR I = 1 TO 10: PRINT RND(6); " ";: NEXT
20 PRINT
30 PRINT RND(1)
40 PRINT INT(RND(1) * 21) - 10
```
<!--ecran norand-->

La ligne 40 donne un entier entre -10 et 10. Chaque `RUN` tire une nouvelle série.

## Exercice

10. Simulez 1000 lancers de deux dés et comptez combien de fois la somme fait 7.

# Lire des données : DATA, READ, RESTORE

Quand un programme a besoin de beaucoup de valeurs fixes (une liste de mots, des questions, des notes de musique…), on les écrit dans des lignes `DATA`. `READ` les lit une à une, dans l'ordre.

```basic
10 FOR I = 1 TO 4
20 READ N$, P
30 PRINT N$; TAB(15); P; " habitants"
40 NEXT
50 DATA Paris, 2087577, Lyon, 520774
60 DATA "Lille", 236710, "Brest", 139619
```
<!--ecran-->

- Les valeurs sont séparées par des virgules. Les lignes `DATA` peuvent être n'importe où dans le programme.
- Un texte peut être écrit avec ou sans guillemets. Sans guillemets, il passe en **majuscules**, comme le reste de la ligne ; avec guillemets, il est gardé tel quel et peut contenir des virgules.
- Lire au-delà de la dernière donnée provoque l'erreur « Plus de DATA ».
- `RESTORE` repart de la première donnée ; `RESTORE n` repart de la ligne `n`.

## Un quiz

```basic
10 CLS: S = 0
20 FOR Q = 1 TO 3
30 READ Q$, R$
40 PRINT Q$
50 INPUT "Réponse"; P$
60 IF UPPER$(P$) = R$ THEN PRINT "Bravo !": S = S + 1 ELSE PRINT "Non : "; R$
70 NEXT
80 PRINT "Score : "; S; "/3"
100 DATA "Capitale de l'Italie ?", ROME
110 DATA "Planète rouge ?", MARS
120 DATA "3 x 7 ?", 21
```
<!--ecran keys="rome\nVenus\n21\n"-->

`UPPER$` permet d'accepter la réponse en minuscules comme en majuscules.

## Choisir une liste avec RESTORE

```basic
10 INPUT "1 = fruits, 2 = légumes"; C
20 ON C GOSUB 100, 200
30 FOR I = 1 TO 3: READ A$: PRINT A$: NEXT
40 END
100 RESTORE 300: RETURN
200 RESTORE 400: RETURN
300 DATA pomme, poire, cerise
400 DATA "carotte", "poireau", "navet"
```
<!--ecran keys="2\n"-->

# Le clavier en direct : INKEY$

`INPUT` arrête le programme jusqu'à ce que l'on appuie sur **ENVOI**. Pour un jeu, il faut au contraire réagir aux touches **sans s'arrêter** : c'est le rôle de `INKEY$`. Il donne la touche frappée, ou une chaîne vide `""` si aucune touche n'a été frappée.

```basic
10 PRINT "Appuyez sur une touche..."
20 K$ = INKEY$: IF K$ = "" THEN 20
30 PRINT "Vous avez tapé : "; K$; " (code "; ASC(K$); ")"
40 IF K$ <> "q" AND K$ <> "Q" THEN 20
50 PRINT "Fin."
```
<!--ecran keys="{WAIT:300}{k:a}{WAIT:300}{k:7}{WAIT:300}{k:q}"-->

- La ligne 20 tourne sur elle-même tant qu'aucune touche n'est frappée : c'est une **attente active**.
- Les lettres arrivent en minuscules ou en majuscules selon l'état du clavier : testez les deux, ou utilisez `UPPER$(K$)`.
- **ENVOI** donne `CHR$(13)`. Les touches de fonction (SUITE, RETOUR…) et les flèches ne sont pas transmises.

## Un objet qui se déplace

Les touches **4** et **6** du pavé numérique déplacent un vaisseau à gauche et à droite ; **ESPACE** quitte.

```basic
10 CLS: X = 20
20 LOCATE X, 12: PRINT " <O> ";
30 K$ = INKEY$
40 IF K$ = "4" AND X > 1 THEN X = X - 1
50 IF K$ = "6" AND X < 35 THEN X = X + 1
60 IF K$ <> " " THEN 20
70 CLS: PRINT "Fin du vol."
```
<!--ecran keys="{WAIT:200}{k:6}{WAIT:200}{k:6}{WAIT:200}{k:6}{WAIT:200}{k:6}{SNAP}{WAIT:200}{k: }" first=8 rows=8-->

L'espace avant et après `<O>` efface l'ancienne position quand le vaisseau bouge d'une case.

# Le son

- `BEEP` fait biper le Minitel.
- `SOUND fréquence, durée` joue un son sur le buzzer de la carte : la fréquence en hertz (de 31 à 20 000), la durée en millisecondes. Le programme attend la fin du son.

```basic
10 FOR F = 200 TO 2000 STEP 200
20 SOUND F, 100
30 NEXT
40 BEEP
```
<!--test-->

## Jouer une mélodie

Les notes de la gamme ont des fréquences précises (le *la* du diapason vaut 440 Hz). On range la mélodie dans des `DATA` : fréquence et durée, et 0 pour finir.

```basic
10 REM Au clair de la lune
20 READ F, D
30 IF F = 0 THEN END
40 SOUND F, D: PAUSE 30
50 GOTO 20
100 DATA 262,400,262,400,262,400,294,400
110 DATA 330,800,294,800,262,400,330,400
120 DATA 294,400,294,400,262,1200,0,0
```
<!--test-->

| Note | do | ré | mi | fa | sol | la | si | do aigu |
|---|---|---|---|---|---|---|---|---|
| Hz | 262 | 294 | 330 | 349 | 392 | 440 | 494 | 523 |

Pour monter d'une octave, on double la fréquence ; pour descendre, on la divise par deux.

# Dessiner

## La grille de points

En plus du texte, le Minitel sait afficher des **caractères semi-graphiques** : chaque case de l'écran est découpée en 2 × 3 petits carrés. reBASIC s'en sert pour offrir une grille de **80 × 72 points** :

- `x` va de 0 (à gauche) à 79 (à droite) ;
- `y` va de 0 (en haut) à 71 (en bas).

| Instruction | Effet |
|---|---|
| `PLOT x, y` | allume le point (x, y) |
| `PLOT x, y, 0` | éteint le point |
| `LINE x1, y1, x2, y2` | trace un segment |
| `LINE x1, y1, x2, y2, 0` | efface un segment |
| `POINT(x, y)` | vaut -1 si le point est allumé, 0 sinon |

```basic
10 CLS
20 LINE 0, 0, 79, 0: LINE 79, 0, 79, 71
30 LINE 79, 71, 0, 71: LINE 0, 71, 0, 0
40 LINE 0, 0, 79, 71: LINE 79, 0, 0, 71
50 GOTO 50
```
<!--ecran rows=24-->

Comme pour `LOCATE`, la ligne 50 laisse le dessin à l'écran jusqu'à ce que vous appuyiez sur **ANNULATION**. Vous remarquerez que le coin en bas à droite n'est pas dessiné : y écrire ferait défiler l'écran.

## Une courbe

```basic
10 CLS
20 LINE 0, 36, 79, 36
30 FOR X = 0 TO 79
40 PLOT X, 36 - 30 * SIN(X / 80 * 6.2832)
50 NEXT
60 GOTO 60
```
<!--ecran rows=24-->

## Un cercle

```basic
10 CLS
20 FOR A = 0 TO 6.3 STEP 0.05
30 PLOT 40 + 30 * COS(A), 36 + 30 * SIN(A)
40 NEXT
50 LOCATE 17, 12: PRINT "rond !";
60 GOTO 60
```
<!--ecran rows=24-->

Le texte et le dessin cohabitent, à condition de ne pas écrire dans une case où l'on dessine encore.

## Couleurs du dessin

`COLOR` change aussi la couleur des points (en gris sur le Minitel 1B). Une case de l'écran n'a qu'une seule couleur : si l'on dessine dans une case déjà occupée, toute la case prend la couleur en cours.

```basic
10 CLS
20 FOR I = 1 TO 7
30 COLOR I
40 FOR Y = I * 9 - 6 TO I * 9: LINE 10, Y, 69, Y: NEXT
50 NEXT
60 GOTO 60
```
<!--ecran rows=24-->

## Une animation : la balle qui rebondit

Pour animer, on efface l'objet, on change sa position, on le redessine.

```basic
10 CLS: X = 5: Y = 5: U = 1: V = 1
20 LINE 0, 0, 79, 0: LINE 0, 71, 78, 71
30 PLOT X, Y, 0
40 X = X + U: Y = Y + V
50 IF X <= 1 OR X >= 78 THEN U = -U
60 IF Y <= 1 OR Y >= 70 THEN V = -V
70 PLOT X, Y
80 GOTO 30
```
<!--ecran rows=24 limit=6000-->

`U` et `V` sont les vitesses horizontale et verticale : quand la balle touche un bord, la vitesse change de signe.

## Lire l'écran : POINT

`POINT` permet de savoir si un point est allumé, par exemple pour détecter une collision. Ce programme trace un serpent qui avance en tournant au hasard, et s'arrête quand il touche un bord ou son propre corps :

```basic
10 CLS: X = 40: Y = 36: U = 1: V = 0: N = 0
20 R = RND(8)
30 IF R = 1 THEN W = U: U = V: V = -W
40 IF R = 2 THEN W = U: U = -V: V = W
50 X = X + U: Y = Y + V
60 IF X < 0 OR X > 79 OR Y < 0 OR Y > 70 THEN 90
70 IF POINT(X, Y) THEN 90
80 PLOT X, Y: N = N + 1: GOTO 20
90 LOCATE 1, 1: PRINT N; " pas.";
100 GOTO 100
```
<!--ecran rows=24-->

`U` et `V` donnent la direction. Une fois sur huit, le serpent tourne d'un côté (ligne 30), une fois sur huit de l'autre (ligne 40), et le reste du temps il va tout droit.

::: {custom-style="Attention"}
**Attention** — reBASIC garde le dessin en mémoire, mais cette mémoire ne suit pas le **défilement** de l'écran. Dessinez toujours après un `CLS`, et placez le texte avec `LOCATE` pour éviter tout défilement.
:::

## Exercices

11. Dessinez un escalier de 8 marches qui descend de gauche à droite.
12. Tracez le graphique de y = x² pour x allant de -8 à 8 (pensez à centrer et à mettre à l'échelle).

# L'heure et le temps

## La date et l'heure

Grâce à l'horloge de la carte rebootMinitel, reBASIC connaît la date et l'heure :

| Fonction | Résultat |
|---|---|
| `TIME$` | l'heure, `"hh:mm:ss"` |
| `DATE$` | la date, `"JJ/MM/AAAA"` |
| `HOUR`, `MINUTE`, `SECOND` | heure, minutes, secondes (nombres) |
| `DAY`, `MONTH`, `YEAR` | jour, mois, année (nombres) |

```basic
10 PRINT "Nous sommes le "; DATE$
20 PRINT "Il est "; TIME$
30 IF HOUR < 12 THEN PRINT "Bonne matinée !" ELSE PRINT "Bon après-midi !"
40 PRINT "Il reste "; 31 - DAY; " jours ce mois-ci."
```
<!--ecran-->

L'heure se règle dans le menu **0 – Réglage Horloge** de l'écran d'accueil. Sans horloge, ces fonctions donnent l'erreur « Horloge absente ».

## Une horloge à l'écran

```basic
10 CLS: LOCATE 13, 10: PRINT DATE$
20 LOCATE 14, 12: PRINT TIME$;
30 PAUSE 200: GOTO 20
```
<!--ecran rows=24 limit=3000-->

## Mesurer une durée : TIMER

`TIMER` donne le nombre de millisecondes écoulées depuis le `RUN`. On l'utilise pour chronométrer :

```basic
10 PRINT "Tapez sur une touche dès que"
20 PRINT "vous voyez le signal."
30 PAUSE 2000 + RND(3000)
40 PRINT ">>> TOP <<<"
50 K$ = INKEY$: T = TIMER
60 K$ = INKEY$: IF K$ = "" THEN 60
70 PRINT "Temps de réaction : "; TIMER - T; " ms"
```
<!--ecran keys="{WAIT:4700}{k:x}"-->

La première lecture de `INKEY$` (ligne 50) vide le clavier, pour ne pas compter une touche tapée trop tôt.

# La carte SD

## Enregistrer ses programmes

| Commande | Effet |
|---|---|
| `SAVE "NOM"` | enregistre le programme dans le fichier `NOM.BAS` |
| `LOAD "NOM"` | charge `NOM.BAS` à la place du programme en cours |
| `MERGE "NOM"` | ajoute les lignes de `NOM.BAS` au programme en cours |
| `FILES` | liste les fichiers de la carte, avec leur taille |
| `KILL "NOM"` | supprime `NOM.BAS` |

Un nom fait 1 à 8 caractères : lettres, chiffres, `_` et `-`. Un fichier qui existe déjà est remplacé sans avertissement.

```basic
NEW
10 PRINT "Je suis sauvé !"
SAVE "SAUVE"
NEW
LIST
LOAD "SAUVE"
LIST
FILES
```
<!--ecran mode=direct-->

::: {custom-style="Astuce"}
**Astuce** — `MERGE` permet de se constituer une bibliothèque de sous-programmes (un titre encadré, un menu…) numérotés à partir de 1000, que l'on ajoute à ses nouveaux programmes.
:::

## Renuméroter : RENUM

`RENUM` renumérote le programme de 10 en 10, et met à jour les numéros qui suivent `GOTO`, `GOSUB`, `THEN`, `ELSE`, `RESTORE` et `ON`. `RENUM 100, 5` commence à 100 avec un pas de 5.

```basic
NEW
1 PRINT "Début"
2 GOSUB 7
3 GOTO 9
7 PRINT "Sous-programme": RETURN
9 PRINT "Fin"
RENUM
LIST
```
<!--ecran mode=direct-->

## Les fichiers de données

Un programme peut aussi écrire et lire ses propres fichiers : un carnet d'adresses, des scores, des mesures… Ce sont des fichiers texte, avec l'extension `.TXT` par défaut.

| Instruction | Effet |
|---|---|
| `OPEN "NOM" FOR OUTPUT AS #1` | crée le fichier (l'efface s'il existe) pour y écrire |
| `OPEN "NOM" FOR APPEND AS #1` | ouvre le fichier pour écrire à la suite |
| `OPEN "NOM" FOR INPUT AS #1` | ouvre le fichier pour le lire |
| `PRINT #1, …` | écrit dans le fichier, comme `PRINT` |
| `INPUT #1, A, B$` | lit des valeurs |
| `LINE INPUT #1, A$` | lit une ligne entière |
| `EOF(1)` | vaut -1 quand tout le fichier a été lu |
| `CLOSE #1` ou `CLOSE` | ferme le fichier (ou tous) |

Deux fichiers peuvent être ouverts en même temps, numéros `#1` et `#2`.

## Écrire puis relire un fichier

```basic
10 OPEN "AMIS" FOR OUTPUT AS #1
20 FOR I = 1 TO 3
30 READ N$, A
40 PRINT #1, N$, A
50 NEXT
60 CLOSE #1
70 OPEN "AMIS" FOR INPUT AS #1
80 WHILE NOT EOF(1)
90 INPUT #1, N$, A
100 PRINT N$; " a "; A; " ans"
110 WEND
120 CLOSE #1
200 DATA "Ali", 34, "Bea", 29, "Chen", 41
```
<!--ecran-->

Dans un fichier, la virgule de `PRINT #` écrit une **virgule** : la ligne écrite est `Ali,34`, et `INPUT #` sait relire les valeurs séparées par des virgules. On peut lire ce fichier sur un ordinateur : c'est un fichier « CSV », que les tableurs savent ouvrir.

## Un journal de bord

`APPEND` ajoute des lignes à la fin d'un fichier existant : chaque `RUN` complète le journal.

```basic
10 INPUT "Note du jour"; N$
20 OPEN "JOURNAL" FOR APPEND AS #1
30 PRINT #1, DATE$; " "; N$
40 CLOSE #1
50 PRINT "--- Journal ---"
60 OPEN "JOURNAL" FOR INPUT AS #1
70 WHILE NOT EOF(1): LINE INPUT #1, L$: PRINT L$: WEND
80 CLOSE
```
<!--ecran keys="Premier essai\n"-->

`LINE INPUT #` lit toute la ligne, virgules comprises. Comme toute chaîne, la ligne lue garde au plus 32 caractères : écrivez des lignes courtes.

::: {custom-style="Astuce"}
**Astuce** — Les fichiers sont fermés automatiquement quand le programme s'arrête, mais prenez l'habitude d'écrire `CLOSE` : c'est à ce moment que tout est réellement écrit sur la carte.
:::

# Piloter des composants électroniques

La carte rebootMinitel est un Arduino : reBASIC peut commander des LED, lire des boutons ou des capteurs branchés sur les broches libres.

| Instruction | Effet |
|---|---|
| `PINMODE b, 1` | la broche `b` devient une **sortie** |
| `PINMODE b, 0` | la broche `b` devient une **entrée** |
| `PINMODE b, 2` | entrée avec **résistance de tirage** interne (pour un bouton) |
| `DWRITE b, v` | met la sortie à 0 V (`v` = 0) ou 5 V (`v` = 1) |
| `DREAD(b)` | lit une entrée : 0 ou 1 |
| `AWRITE b, v` | sortie « analogique » (MLI) de 0 à 255 |
| `AREAD(a)` | lit l'entrée analogique `a` (A0 à A15) : de 0 à 1023 |

Les broches qui servent déjà au Minitel, à la carte SD, à l'horloge et au buzzer sont protégées : les utiliser donne l'erreur « Broche interdite ».

## Faire clignoter une LED

Branchez une LED et une résistance de 220 Ω en série entre la broche 8 et la masse (GND).

```basic
10 PINMODE 8, 1
20 FOR I = 1 TO 10
30 DWRITE 8, 1: PAUSE 300
40 DWRITE 8, 0: PAUSE 300
50 NEXT
```
<!--test-->

## Lire un bouton

Branchez un bouton-poussoir entre la broche 7 et la masse. En mode 2, la broche est à 1 quand le bouton est relâché et passe à 0 quand on appuie.

```basic
10 PINMODE 7, 2
20 CLS: N = 0
30 IF DREAD(7) = 1 THEN 30
40 N = N + 1: LOCATE 1, 1: PRINT "Appuis : "; N;
50 IF DREAD(7) = 0 THEN 50
60 GOTO 30
```
<!--test limit=20000-->

La ligne 50 attend que le bouton soit relâché, pour ne compter qu'un appui à la fois.

## Lire un potentiomètre

Un potentiomètre branché entre 5 V et GND, son curseur sur l'entrée A0, donne une valeur de 0 à 1023. Le programme l'affiche sous forme de barre :

```basic
10 CLS
20 V = AREAD(0)
30 LOCATE 1, 10: PRINT "Valeur : "; V; "    "
40 L = INT(V / 1024 * 80)
50 LINE 0, 33, L, 33: LINE L + 1, 33, 79, 33, 0
60 PAUSE 100: GOTO 20
```
<!--ecran rows=12 limit=3000-->

## Varier la lumière d'une LED

Sur les broches marquées « ~ » ou PWM de l'Arduino Mega (3 à 13, 44 à 46), `AWRITE` règle la luminosité d'une LED de 0 (éteinte) à 255 (pleine puissance) :

```basic
10 FOR V = 0 TO 255 STEP 5: AWRITE 9, V: PAUSE 20: NEXT
20 FOR V = 255 TO 0 STEP -5: AWRITE 9, V: PAUSE 20: NEXT
30 GOTO 10
```
<!--test limit=3000-->

# Comprendre les erreurs

## Lire un message d'erreur

Quand reBASIC rencontre un problème, il s'arrête et affiche trois informations : **ce qui ne va pas**, le **numéro de la ligne**, et la ligne elle-même avec l'endroit du problème en **vidéo inverse**.

```basic
10 A = 5
20 B = 0
30 PRINT A / B
```
<!--ecran erreur-->

## Les messages et leurs causes

| Message | Cause probable |
|---|---|
| Erreur de syntaxe | mot mal orthographié, parenthèse ou guillemet oublié, nom de variable de deux lettres, mot-clé collé à une variable (`PRINTA`) |
| Division par zéro | une division ou un `MOD` par 0 |
| Ligne *n* inexistante | `GOTO`, `GOSUB` ou `RESTORE` vers une ligne qui n'existe pas |
| RETURN sans GOSUB | le programme est « tombé » dans un sous-programme : un `END` manque |
| NEXT sans FOR | un `NEXT` de trop, ou une boucle mal imbriquée |
| WEND sans WHILE, WHILE sans WEND | une boucle `WHILE` incomplète |
| Type incompatible | un nombre là où il faut une chaîne, ou l'inverse (`A = "OUI"`) |
| Indice hors limites | indice de tableau négatif ou plus grand que prévu par `DIM` |
| Valeur incorrecte | paramètre hors limites (`LOCATE 50,1`, `COLOR 9`…) |
| Mémoire pleine | programme ou tableaux trop grands |
| Trop d'imbrications | trop de parenthèses, de `GOSUB`, `FOR` ou `WHILE` imbriqués |
| Plus de DATA | un `READ` de trop |
| Fichier introuvable, Fichier non ouvert… | problème de fichier sur la carte SD |
| Interruption | vous avez appuyé sur ANNULATION ou SOMMAIRE |

## Chercher une erreur de logique

Le programme ne s'arrête pas en erreur, mais il ne fait pas ce que vous voulez ? Quelques méthodes :

- **Affichez** les variables à des endroits clés, avec un `PRINT` provisoire.
- Après l'arrêt du programme, ses variables sont toujours là : affichez-les en mode direct.
- Ajoutez un `STOP` pour arrêter le programme à un endroit précis et examiner les variables.
- `GOTO n` en mode direct relance le programme à la ligne `n` **sans** effacer les variables, contrairement à `RUN`.

```basic
10 S = 0
20 FOR I = 1 TO 5: S = S + I * I: NEXT
30 PRINT "Somme des carrés : "; S
RUN
PRINT I, S
```
<!--ecran mode=direct-->

On voit qu'après la boucle, `I` vaut 6 : la boucle s'arrête quand `I` **dépasse** 5.

# Projets

Voici quelques programmes plus complets qui combinent tout ce que vous avez appris. Tapez-les, enregistrez-les avec `SAVE`, puis modifiez-les à votre goût.

## Le pendu

```basic
10 CLS: RESTORE: N = RND(5)
20 FOR I = 1 TO N: READ M$: NEXT
30 L = LEN(M$): T$ = "": E = 0
40 FOR I = 1 TO L: T$ = T$ + "-": NEXT
50 PRINT: PRINT T$; "   erreurs : "; E; "/7"
60 IF T$ = M$ THEN PRINT "Gagné !": END
70 IF E = 7 THEN PRINT "Perdu, c'était "; M$: END
80 INPUT "Lettre"; C$: C$ = UPPER$(LEFT$(C$, 1))
90 IF INSTR(M$, C$) = 0 THEN E = E + 1: GOTO 50
100 FOR I = 1 TO L
110 IF MID$(M$, I, 1) = C$ THEN GOSUB 200
120 NEXT: GOTO 50
200 T$ = LEFT$(T$, I - 1) + C$ + MID$(T$, I + 1): RETURN
300 DATA MINITEL, ARDUINO, CLAVIER, MODEM, ECRAN
```
<!--ecran keys="e\na\nm\ni\no\nd\n"-->

La ligne 200 reconstruit le mot affiché en remplaçant le `I`-ième tiret par la lettre trouvée.

## Le calcul mental chronométré

```basic
10 CLS: S = 0: T = TIMER
20 FOR Q = 1 TO 5
30 A = RND(9) + 1: B = RND(9) + 1
40 PRINT A; " x "; B; " = ";: INPUT R
50 IF R = A * B THEN S = S + 1 ELSE PRINT "Non, "; A * B
60 NEXT
70 T = INT((TIMER - T) / 100) / 10
80 PRINT S; "/5 en "; T; " secondes"
```
<!--ecran keys="{WAIT:2000}27\n{WAIT:3000}18\n{WAIT:1500}63\n{WAIT:4000}16\n{WAIT:2500}24\n"-->

`INPUT` sans texte, après un `PRINT` terminé par `;`, pose la question sur la même ligne.

## Un histogramme des températures

```basic
10 CLS: PRINT "Températures de la semaine"
20 FOR J = 1 TO 7
30 READ J$, T
40 PRINT J$; TAB(5); T; TAB(9);
50 FOR I = 1 TO T: PRINT "#";: NEXT
60 PRINT
70 NEXT
100 DATA Lun,12,Mar,15,Mer,18,Jeu,21
110 DATA Ven,17,Sam,24,Dim,20
```
<!--ecran-->

## Le jeu de réflexes

Une cible apparaît au hasard ; il faut taper le chiffre qu'elle affiche le plus vite possible.

```basic
10 CLS: S = 0
20 FOR Q = 1 TO 5
30 X = RND(36): Y = RND(20) + 2: N = RND(9)
40 LOCATE X, Y: INVERSE 1: PRINT " "; N; " ";: INVERSE 0
50 T = TIMER
60 K$ = INKEY$: IF K$ = "" THEN 60
70 D = TIMER - T: LOCATE X, Y: PRINT "   ";
80 IF VAL(K$) = N THEN S = S + 2000 - D
90 NEXT
100 LOCATE 1, 1: PRINT "Score : "; INT(S);
110 GOTO 110
```
<!--ecran keys="{WAIT:500}{k:1}{WAIT:700}{k:8}{WAIT:400}{k:7}{WAIT:600}{k:5}{WAIT:500}{k:6}" rows=3-->

## Le dessin à main levée

Les touches **8**, **2**, **4**, **6** déplacent le crayon (haut, bas, gauche, droite), **5** lève ou baisse le crayon, **E** efface l'écran.

```basic
10 CLS: X = 40: Y = 36: C = 1
20 K$ = UPPER$(INKEY$): IF K$ = "" THEN 20
30 IF K$ = "8" AND Y > 0 THEN Y = Y - 1
40 IF K$ = "2" AND Y < 70 THEN Y = Y + 1
50 IF K$ = "4" AND X > 0 THEN X = X - 1
60 IF K$ = "6" AND X < 79 THEN X = X + 1
70 IF K$ = "5" THEN C = 1 - C
80 IF K$ = "E" THEN CLS
90 IF C = 1 THEN PLOT X, Y
100 GOTO 20
```
<!--ecran keys="{k:6}{k:6}{k:6}{k:6}{k:6}{k:6}{k:6}{k:6}{k:6}{k:6}{k:2}{k:2}{k:2}{k:2}{k:2}{k:2}{k:2}{k:2}{k:2}{k:2}{k:4}{k:4}{k:4}{k:4}{k:4}{k:4}{k:4}{k:4}{k:4}{k:4}{k:8}{k:8}{k:8}{k:8}{k:8}{k:8}{k:8}{k:8}{k:8}{k:8}" rows=24 limit=3000-->

# Corrigés des exercices

Il y a toujours plusieurs façons d'écrire un programme : si le vôtre fonctionne, il est juste !

**Exercice 1**

```basic
10 INPUT "Premier nombre"; A
20 INPUT "Second nombre"; B
30 PRINT "Somme : "; A + B
40 PRINT "Différence : "; A - B
50 PRINT "Produit : "; A * B
60 IF B = 0 THEN PRINT "Pas de division par 0": END
70 PRINT "Quotient : "; A / B
```
<!--test keys="12\n4\n"-->

**Exercice 2**

```basic
10 P = 3.14159
20 INPUT "Rayon"; R
30 PRINT "Périmètre : "; 2 * P * R
40 PRINT "Surface : "; P * R ^ 2
```
<!--test keys="5\n"-->

**Exercice 3**

```basic
10 INPUT "Note sur 20"; N
20 IF N >= 16 THEN PRINT "Très bien": END
30 IF N >= 12 THEN PRINT "Bien": END
40 IF N >= 10 THEN PRINT "Passable": END
50 PRINT "Insuffisant"
```
<!--test keys="13\n"-->

**Exercice 4**

```basic
10 INPUT "Année"; A
20 B = (A MOD 4 = 0 AND A MOD 100 <> 0) OR A MOD 400 = 0
30 IF B THEN PRINT "Bissextile" ELSE PRINT "Pas bissextile"
```
<!--test keys="2024\n"-->

La ligne 20 range dans `B` la valeur de la condition : -1 (vraie) ou 0 (fausse).

**Exercice 5**

```basic
10 FOR I = 1 TO 12
20 PRINT I; " x "; I; " = "; I * I
30 NEXT
```
<!--test-->

**Exercice 6**

```basic
10 C = 1000: A = 0
20 WHILE C < 2000
30 C = C * 1.03: A = A + 1
40 WEND
50 PRINT "Doublé au bout de "; A; " ans"
```
<!--test-->

**Exercice 7**

```basic
10 FOR L = 5 TO 1 STEP -1
20 FOR C = 1 TO L: PRINT "*";: NEXT C
30 PRINT
40 NEXT L
```
<!--test-->

**Exercice 8** — le « tri à bulles »

```basic
10 DIM T(10)
20 FOR I = 1 TO 10: T(I) = RND(50): PRINT T(I); " ";: NEXT
30 PRINT
40 E = 0
50 FOR I = 1 TO 9
60 IF T(I) > T(I + 1) THEN GOSUB 200
70 NEXT
80 IF E = 1 THEN 40
90 FOR I = 1 TO 10: PRINT T(I); " ";: NEXT
100 PRINT: END
200 X = T(I): T(I) = T(I + 1): T(I + 1) = X: E = 1: RETURN
```
<!--ecran norand-->

L'échange se fait dans un sous-programme, car le `NEXT` de la ligne 70 ne doit pas se trouver après le `IF`.

**Exercice 9**

```basic
10 INPUT "Phrase"; P$
20 P$ = UPPER$(P$): N = 0
30 FOR I = 1 TO LEN(P$)
40 IF INSTR("AEIOUY", MID$(P$, I, 1)) THEN N = N + 1
50 NEXT
60 PRINT N; " voyelles"
```
<!--test keys="Bonjour le Minitel\n"-->

**Exercice 10**

```basic
10 N = 0
20 FOR I = 1 TO 1000
30 IF RND(6) + RND(6) = 7 THEN N = N + 1
40 NEXT
50 PRINT "Somme 7 : "; N; " fois sur 1000"
```
<!--test-->

On trouve environ 167 : il y a 6 façons sur 36 de faire 7.

**Exercice 11**

```basic
10 CLS
20 FOR M = 0 TO 7
30 X = M * 10: Y = M * 8 + 4
40 LINE X, Y, X + 10, Y: LINE X + 10, Y, X + 10, Y + 8
50 NEXT
60 GOTO 60
```
<!--ecran rows=24-->

**Exercice 12**

```basic
10 CLS: LINE 0, 68, 79, 68: LINE 40, 0, 40, 70
20 FOR X = -8 TO 8 STEP 0.1
30 PLOT 40 + X * 4.5, 68 - X * X
40 NEXT
50 GOTO 50
```
<!--ecran rows=24-->

# Aide-mémoire

## Touches

| Touche | Effet |
|---|---|
| ENVOI | valide la ligne |
| CORRECTION | efface le dernier caractère |
| ANNULATION | abandonne la ligne ; interrompt le programme |
| GUIDE | affiche l'aide (SUITE, RETOUR, ENVOI pour revenir) |
| SOMMAIRE | interrompt le programme ; quitte reBASIC |

## Commandes

`RUN` · `LIST [début][-fin]` · `NEW` · `MEM` · `RENUM [début][,pas]` · `AIDE` · `SAVE "NOM"` · `LOAD "NOM"` · `MERGE "NOM"` · `FILES` · `KILL "NOM"`

## Instructions

| Domaine | Instructions |
|---|---|
| Affichage | `PRINT` (`?`) avec `;` `,` `TAB(n)` · `CLS` · `LOCATE col,rang` · `COLOR n` · `INVERSE n` |
| Saisie | `INPUT ["texte";] v,…` |
| Variables | `[LET] v = expr` · `DIM A(n)` |
| Décisions | `IF … THEN … [ELSE …]` · `GOTO n` · `ON x GOTO n1,…` |
| Boucles | `FOR v=a TO b [STEP c]` … `NEXT [v]` · `WHILE cond` … `WEND` |
| Sous-programmes | `GOSUB n` · `RETURN` · `ON x GOSUB n1,…` |
| Données | `DATA` · `READ v,…` · `RESTORE [n]` |
| Fin, commentaire | `END` · `STOP` · `REM` · `'` |
| Son, attente | `BEEP` · `SOUND f,ms` · `PAUSE ms` |
| Dessin | `PLOT x,y[,c]` · `LINE x1,y1,x2,y2[,c]` |
| Fichiers | `OPEN "N" FOR INPUT/OUTPUT/APPEND AS #n` · `PRINT #n` · `INPUT #n` · `LINE INPUT #n` · `CLOSE` |
| Broches | `PINMODE b,m` · `DWRITE b,v` · `AWRITE b,v` |

## Fonctions

| Domaine | Fonctions |
|---|---|
| Mathématiques | `ABS` `INT` `SGN` `SQR` `SIN` `COS` `TAN` `ATN` `LOG` `EXP` `RND(n)` |
| Chaînes | `LEN` `LEFT$` `RIGHT$` `MID$` `INSTR` `UPPER$` `LOWER$` `VAL` `STR$` `ASC` `CHR$` `HEX$` |
| Clavier | `INKEY$` |
| Temps | `TIMER` `TIME$` `DATE$` `HOUR` `MINUTE` `SECOND` `DAY` `MONTH` `YEAR` |
| Dessin | `POINT(x,y)` |
| Fichiers | `EOF(n)` |
| Broches | `DREAD(b)` `AREAD(a)` |

## Opérateurs

| Type | Opérateurs |
|---|---|
| Calcul | `+` `-` `*` `/` `^` `MOD` |
| Comparaison | `=` `<>` `<` `>` `<=` `>=` |
| Logique | `AND` `OR` `NOT` (vrai = -1, faux = 0) |
| Chaînes | `+` (concaténation) |

## Limites sur l'Arduino Mega

| Élément | Limite |
|---|---|
| Taille du programme | 2 048 octets |
| Longueur d'une ligne | 78 caractères |
| Longueur d'une chaîne | 32 caractères (lettre accentuée = 2) |
| Variables | `A`–`Z`, `A$`–`Z$`, tableaux `A()`–`Z()` |
| Cases de tableaux | 64 au total |
| Imbrications | 12 `GOSUB`, 8 `FOR`, 6 `WHILE`, 6 niveaux de parenthèses |
| Écran texte | 40 colonnes × 24 rangées |
| Écran graphique | 80 × 72 points |
| Fichiers ouverts | 2 |
