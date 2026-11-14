#!/bin/sh
# Compile le simulateur à partir du Basic.ino du croquis (deux dossiers plus haut).
# Usage : ./compiler.sh [--verifier]   (--verifier ajoute les contrôles de mémoire d'AddressSanitizer)
set -e
ICI=$(cd "$(dirname "$0")" && pwd)
mkdir -p "$ICI/build"
python3 - "$ICI" <<'PY'
import re, sys, os
ici = sys.argv[1]
src = open(os.path.join(ici, '..', '..', 'Basic.ino'), encoding='utf-8').read()
# Compteur d'erreurs pour le simulateur
src = src.replace('void basPrintError()\n{', 'void basPrintError()\n{\n  g_erreurs++;', 1)
open(os.path.join(ici, 'build', 'Basic.cpp'), 'w', encoding='utf-8').write(src)
# Prototypes : l'IDE Arduino les fabrique automatiquement, ici on le fait nous-mêmes
protos = []
for m in re.finditer(r'^((?:BAS_NOINLINE )?[A-Za-z_][\w ]*?[\w\*&]+\s*\**\s*bas\w+\([^;{]*\))\s*\n\{', src, re.M):
    protos.append(m.group(1).replace('BAS_NOINLINE ', '') + ';')
open(os.path.join(ici, 'build', 'prototypes.h'), 'w').write(
    '#define BAS_NOINLINE __attribute__((noinline))\nfloat basExpr();\nvoid basStatement();\n' + '\n'.join(protos) + '\n')
PY
OPTIONS="-O2"
if [ "$1" = "--verifier" ]; then OPTIONS="-O1 -g -fsanitize=address,undefined"; fi
${CXX:-g++} -std=c++17 $OPTIONS -w -I"$ICI" -I"$ICI/build" -o "$ICI/build/simulateur" "$ICI/simulateur.cpp"
echo "Simulateur compilé : $ICI/build/simulateur"
