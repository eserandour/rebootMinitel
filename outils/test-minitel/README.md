# Test des zones sur un vrai Minitel

Le simulateur C et le Minitel virtuel imitent le Minitel 1B : ils doivent savoir comment il applique ses **attributs de zone** (le fond et le soulignement), qui ne sont décrits qu'en partie dans le STUM. Une photo d'un Minitel 1B Alcatel a montré, le 19/11/2026, que le fond d'une case vide dépend du dernier délimiteur à sa gauche **au moment de l'affichage** : récrire une case semi-graphique change le fond des cases qui la suivent. `ZONES.BAS` vérifie cette règle et trois autres, qui n'ont pas encore été vues sur un vrai Minitel.

## Utilisation

1. Copiez `ZONES.BAS` sur la carte SD (ou passez par la [passerelle](../passerelle/) : `LOAD "PC:ZONES"`).
2. Dans reBASIC : `LOAD "ZONES"`, puis `RUN`.
3. Pour chaque test, notez ce que vous voyez : **A** ou **B**. `ANNULATION` arrête le programme.

Le programme écrit les séquences Vidéotex lui-même (`CHR$(27)` : ESC, `CHR$(31)` : US, `CHR$(14)` et `CHR$(15)` : passage en semi-graphique et retour au texte), pour ne pas dépendre de reBASIC.

## Les tests

| Test | Ce qui est envoyé | A | B | Simulateurs |
|---|---|---|---|---|
| 1 Fond | Rangée 3 : un pavé semi-graphique vide sur fond blanc en colonne 12, puis un semi-graphique sur fond noir en colonne 26 | Blanc de 12 à 25 : les cases vides prennent le fond du délimiteur à leur gauche | Seule la case 12 est blanche | A (vu sur un Minitel 1B) |
| 2 Recalcul | Rangée 6 : « XYZ » en colonne 16 et un semi-graphique noir en 22, **puis** le pavé blanc en 12 | « XYZ » disparaît (blanc sur blanc) : le fond est recalculé après coup | « XYZ » reste visible | A (vu sur un Minitel 1B) |
| 3 Souligné | Rangée 9 : « TEXTE » en colonne 16, **puis** en colonne 12 un début de soulignement suivi d'un espace (délimiteur) | « TEXTE » devient souligné | « TEXTE » n'est pas souligné | B (à vérifier) |
| 4 Efface | Rangée 12 : un espace sur fond blanc (délimiteur) en colonne 12, puis, fond noir demandé, l'effacement de fin de rangée (CAN) en colonne 24 | Noir à partir de 24 | Blanc jusqu'au bout | A (à vérifier) |
| 5 Position | Vidéo inverse, puis un positionnement US et « US » ; vidéo inverse, puis un positionnement CSI (celui de `LOCATE`) et « CSI » | Le texte reste en vidéo inverse : le positionnement garde les attributs | Le texte est normal : le positionnement remet les attributs à zéro | A pour les deux (à vérifier) |

Si vos réponses diffèrent de la dernière colonne, le simulateur (`outils/simulateur/imitations.h`, classe `Minitel`) et le Minitel virtuel (`outils/minitel-virtuel/rebasic.js`, classe `Ecran`) doivent être corrigés de la même façon, et le moteur de dessin de reBASIC peut-être aussi.
