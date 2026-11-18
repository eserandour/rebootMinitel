# Remplace les caractères non ASCII des chaînes de rebasic.js par des \uXXXX,
# pour que la bibliothèque marche même dans une page qui n'est pas en UTF-8.
import sys
p = sys.argv[1]; s = open(p, encoding='utf-8').read()
out = []; i = 0; n = len(s); etat = 'code'; q = ''
while i < n:
    c = s[i]
    if etat == 'code':
        if s.startswith('//', i):
            j = s.find('\n', i); j = n if j < 0 else j
            out.append(s[i:j]); i = j; continue
        if s.startswith('/*', i):
            j = s.find('*/', i) + 2; out.append(s[i:j]); i = j; continue
        if c in '\'"`':
            etat = 'chaine'; q = c
        out.append(c); i += 1; continue
    if c == '\\': out.append(s[i:i+2]); i += 2; continue
    if c == q: etat = 'code'
    out.append('\\u%04x' % ord(c) if ord(c) > 127 else c); i += 1
open(p, 'w', encoding='utf-8').write(''.join(out))
