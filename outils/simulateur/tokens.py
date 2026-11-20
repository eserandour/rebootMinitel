"""La place qu'occupe un programme dans la mémoire de reBASIC.

Sur la carte, chaque mot-clé du programme est rangé sous la forme d'un code (« token »)
d'un octet (les 64 premiers de TOK_NOMS) ou de deux octets (les suivants) : voir
basTokenize dans Basic.ino. taille_tokenisee() reprend ce calcul, pour vérifier qu'un
programme tient dans la mémoire de l'Arduino Mega sans avoir à le lancer.
"""

# Comme basTokNoms (Basic.ino) et TOK_NOMS (rebasic.js), dans le même ordre
TOK_NOMS = [
    'PRINT', 'INPUT', 'IF', 'THEN', 'ELSE', 'GOTO', 'GOSUB', 'RETURN',
    'FOR', 'TO', 'STEP', 'NEXT', 'WHILE', 'WEND', 'DATA', 'READ',
    'RESTORE', 'DIM', 'REM', 'END', 'STOP', 'CLS', 'LOCATE', 'PAUSE',
    'SOUND', 'BEEP', 'PLOT', 'LINE', 'BOX', 'CIRCLE', 'COLOR', 'PAPER',
    'INVERSE', 'FLASH', 'UNDERLINE', 'SIZE', 'AND', 'OR', 'NOT', 'MOD',
    'INT', 'RND', 'LEN', 'CHR$', 'STR$', 'LEFT$', 'RIGHT$', 'MID$',
    'INKEY$', 'KEY', 'POINT', 'TIMER', 'TIME$', 'DATE$', 'ASC', 'VAL',
    'INSTR', 'TAB', 'ABS', 'SQR', 'SIN', 'COS', 'ON', 'VDT',
    'CHAIN', 'SGN', 'TAN', 'ATN', 'LOG', 'EXP', 'HOUR', 'MINUTE',
    'SECOND', 'DAY', 'MONTH', 'YEAR', 'DREAD', 'AREAD', 'EOF', 'HEX$',
    'UPPER$', 'LOWER$', 'OPEN', 'CLOSE', 'OUTPUT', 'APPEND', 'PINMODE', 'DWRITE',
    'AWRITE', 'LET', 'RUN', 'LIST', 'NEW', 'MEM', 'RENUM', 'AIDE',
    'HELP', 'SAVE', 'LOAD', 'MERGE', 'FILES', 'KILL', 'EDIT',
]
TOK_UN = 64

# Mémoire du programme et des tableaux sur l'Arduino Mega (BAS_MEM_SIZE)
MEMOIRE_MEGA = 2304


def majuscules(s):
    """Les minuscules en majuscules, sauf entre guillemets (comme basUpper)."""
    g, out = False, []
    for ch in s:
        if ch == '"':
            g = not g
        elif not g and 'a' <= ch <= 'z':
            ch = ch.upper()
        out.append(ch)
    return ''.join(out)


def _lettre(ch):
    return 'A' <= ch <= 'Z'


def taille_tokenisee(s):
    """Nombre d'octets du texte s (en majuscules) une fois tokenisé."""
    n, i = 0, 0
    brut = donnees = guillemets = False
    while i < len(s):
        ch = s[i]
        if ord(ch) >= 0x80:
            n += len(ch.encode('utf-8'))
            i += 1
            continue
        if ch == '"':
            guillemets = not guillemets
        if guillemets or brut or ch == '"':
            n += 1; i += 1
            continue
        if donnees:
            if ch == ':':
                donnees = False
            n += 1; i += 1
            continue
        if ch == "'":
            brut = True
            n += 1; i += 1
            continue
        if not _lettre(ch):
            n += 1; i += 1
            continue
        # Début d'un mot
        if s.startswith('REM', i):
            brut = True
        elif s.startswith('DATA', i):
            donnees = True
        trouve = -1
        for k, nom in enumerate(TOK_NOMS):
            if not s.startswith(nom, i):
                continue
            if _lettre(nom[-1]) and i + len(nom) < len(s) and _lettre(s[i + len(nom)]):
                continue
            trouve = k
            break
        if trouve < 0:
            while i < len(s) and _lettre(s[i]):
                n += 1; i += 1
            continue
        n += 1 if trouve < TOK_UN else 2
        i += len(TOK_NOMS[trouve])
    return n


def taille_ligne(texte):
    """Place d'une ligne de programme (sans son numéro) : numéro, longueur, texte, zéro."""
    return 3 + taille_tokenisee(majuscules(texte).rstrip(' ')) + 1
