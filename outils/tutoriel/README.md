# Fabrication du tutoriel reBASIC

`tutoriel.md` est la source du tutoriel « Programmer en reBASIC ». `./fabriquer.sh` en fait un document LibreOffice et un PDF :

```sh
./fabriquer.sh
# → sortie/Tutoriel_reBASIC.odt et sortie/Tutoriel_reBASIC.pdf
```

Chaque exemple du tutoriel est **exécuté dans le simulateur** (`../simulateur`), avec le `Basic.ino` du croquis. Les images d'écran du document sont donc toujours à jour : si reBASIC change, il suffit de relancer `./fabriquer.sh`. La fabrication s'arrête si un exemple produit une erreur inattendue ou dépasse les limites de l'Arduino Mega (lignes de 78 caractères, chaînes de 32 octets, programmes de 2304 octets, mots-clés tokenisés compris).

## Ce qu'il faut

- ce qu'il faut pour le simulateur : un compilateur C++, Python 3 et Pillow ;
- [pandoc](https://pandoc.org/) ;
- LibreOffice, avec son module Python (`python3-uno` sous Debian et Ubuntu) ;
- le module Python `lxml`.

## Les étapes

| Étape | Script |
|---|---|
| 1. Exécute les exemples, vérifie les limites, produit les images et `sortie/tutoriel_final.md` | `executer_exemples.py` |
| 2. Construit les styles du document (titres, blocs de code, encadrés, A4) | `styles_odt.py` |
| 3. Convertit en ODT | `pandoc` |
| 4. Ajuste l'espacement des blocs de code et les attache à leur image | `blocs_de_code.py` |
| 5. Met à jour le sommaire, met en forme les tableaux, exporte le PDF | `libreoffice.py` |

`sortie/rapport.txt` contient, pour chaque exemple, le programme et l'écran obtenu : pratique pour relire les résultats.

## Écrire dans tutoriel.md

C'est du Markdown (version pandoc). Les programmes sont dans des blocs `basic`, suivis d'un marqueur qui dit quoi en faire :

````markdown
```basic
10 INPUT "Votre âge"; A
20 PRINT "Dans 10 ans : "; A + 10
```
<!--ecran keys="33\n"-->
````

| Marqueur | Effet |
|---|---|
| `<!--ecran-->` | exécute le programme (`RUN`) et insère l'image de l'écran |
| `<!--test-->` | exécute sans image (un bloc sans marqueur est aussi testé) |
| `mode=direct` | les lignes sont tapées telles quelles, en mode direct (avec `LIST`, `RUN`…) |
| `keys="…"` | touches tapées après `RUN` (même écriture que pour le simulateur) |
| `rows=N`, `first=N`, `last=N` | cadrage de l'image |
| `limit=N` | attentes avant d'interrompre une boucle sans fin |
| `erreur` | une erreur est attendue (exemple qui montre un message d'erreur) |
| `pc` | la [passerelle](../passerelle/) est branchée, avec le dossier `sortie/pc` (pour `SAVE "PC:NOM"`…) |

Les encadrés s'écrivent ainsi :

```markdown
::: {custom-style="Astuce"}
**Astuce** — …
:::
```

(ou `custom-style="Attention"`).
