#!/bin/sh
# Fabrique le tutoriel : sortie/Tutoriel_reBASIC.odt et sortie/Tutoriel_reBASIC.pdf
# Il faut : g++, python3 (Pillow, lxml), pandoc, LibreOffice (avec le module Python « uno »)
set -e
cd "$(dirname "$0")"
python3 executer_exemples.py
python3 styles_odt.py
pandoc sortie/tutoriel_final.md -f markdown -t odt --toc --toc-depth=2 -M toc-title="Sommaire" \
  --reference-doc=sortie/reference.odt --resource-path=sortie -o sortie/etape1.odt
python3 blocs_de_code.py sortie/etape1.odt sortie/etape2.odt
python3 libreoffice.py sortie/etape2.odt sortie/Tutoriel_reBASIC.odt sortie/Tutoriel_reBASIC.pdf
rm -f sortie/etape1.odt sortie/etape2.odt
echo "Terminé : sortie/Tutoriel_reBASIC.odt et sortie/Tutoriel_reBASIC.pdf"
