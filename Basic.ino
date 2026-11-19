//////////////////////////////////////////////////////////////////////////////////////

// BASIC

//////////////////////////////////////////////////////////////////////////////////////
/*
  Un petit interpréteur BASIC pour le Minitel.

  Touches :
    ENVOI (ou Retour chariot) : valide la ligne
    CORRECTION                : efface le dernier caractère
    ANNULATION                : abandonne la ligne / interrompt le programme
    GUIDE                     : affiche l'aide (SUITE et RETOUR changent de page, ENVOI la quitte)
    SOMMAIRE                  : interrompt le programme / revient au menu

  Une ligne qui commence par un numéro (1 à 65535) est rangée dans le programme.
  Un numéro seul supprime la ligne. Sans numéro, la ligne est exécutée tout de suite.
  Plusieurs instructions peuvent se suivre sur une ligne, séparées par ":".
  Les minuscules sont converties en majuscules (sauf entre guillemets).

  Commandes :
    RUN   LIST [début][-fin] [/P]   NEW   MEM   AIDE
    LIST /P              : s'arrête à chaque écran plein (SUITE continue, ANNULATION arrête)
    EDIT n               : réaffiche la ligne n dans la zone de saisie pour la corriger
    RENUM [début][,pas]  : renumérote le programme (10,10 par défaut) et met à jour
                           les numéros après GOTO, GOSUB, THEN, ELSE, RESTORE et ON...
    SAVE "NOM"   LOAD "NOM"   MERGE "NOM"   FILES   KILL "NOM"
    (programmes NOM.BAS sur la carte SD ; MERGE ajoute les lignes au programme en cours)
    SAVE "PC:NOM"   LOAD "PC:NOM"   MERGE "PC:NOM"   FILES "PC:"
    (programmes rangés sur l'ordinateur, relié par le câble USB : voir basPcSave)

  Instructions :
    PRINT (ou ?) avec ; et , et TAB(n)     INPUT ["texte";] variable[,variable...]
    [LET] variable = expression
    IF condition THEN instruction(s) ou numéro [ELSE instruction(s) ou numéro]
    GOTO n     GOSUB n     RETURN          ON x GOTO n1,n2...   ON x GOSUB n1,n2...
    FOR v = a TO b [STEP c]     NEXT [v]   WHILE condition ... WEND
    DATA 12,"TEXTE",-3.5   READ variable[,variable...]   RESTORE [n]
    DIM A(n)   REM         END   STOP
    CLS        LOCATE colonne,rangée       BEEP   SOUND fréquence,durée(ms)   PAUSE durée(ms)
    PLOT x,y[,c]   LINE x1,y1,x2,y2[,c]    BOX x1,y1,x2,y2[,c[,p]]   CIRCLE x,y,r[,c[,p]]
    (dessin, voir plus bas)

  Attributs d'affichage (0 à 7 : noir, rouge, vert, jaune, bleu, magenta, cyan, blanc) :
    COLOR n      : couleur des caractères et des points
    PAPER n      : couleur de fond. C'est un attribut de zone du Minitel : en texte, il
                   prend effet au premier espace écrit ensuite et dure jusqu'à la fin de la
                   rangée (ou jusqu'au prochain espace après un autre PAPER). En dessin,
                   chaque case semi-graphique prend aussitôt le fond en cours.
    INVERSE n    : vidéo inverse (1) ou normale (0)
    FLASH n      : clignotement (1) ou fixe (0)
    UNDERLINE n  : soulignement (1) ou non (0). Attribut de zone comme PAPER : il commence
                   au premier espace qui suit.
    SIZE n       : 0 normale, 1 double hauteur, 2 double largeur, 3 double taille. En
                   double hauteur, le curseur descend d'une rangée et le texte occupe la
                   rangée de départ et celle-ci (sans effet sur la rangée 1).
    CLS, la fin du programme et les messages d'erreur reviennent à l'affichage normal.
    Le semi-graphique annule la taille, l'inversion et le soulignement du Minitel :
    reBASIC les rétablit après chaque dessin.

  Fichiers de données (carte SD, extension .TXT par défaut, 2 fichiers ouverts au plus) :
    OPEN "NOM" FOR INPUT|OUTPUT|APPEND AS #n   (n = 1 ou 2)
    PRINT #n, ...      INPUT #n, variable[,variable...]     LINE INPUT #n, A$
    CLOSE [#n]         EOF(n) : -1 à la fin du fichier
    Avec PRINT #, la virgule écrit une virgule dans le fichier : INPUT # lit les valeurs
    séparées par des virgules ou des fins de ligne. LINE INPUT # lit une ligne entière.
    Les fichiers sont fermés par CLOSE, RUN, NEW, LOAD, quand le programme s'arrête
    (fin, END, STOP, erreur, interruption) et en quittant le BASIC.

  Broches de l'Arduino (les broches utilisées par le Minitel, la carte SD, l'horloge
  et le buzzer sont refusées) :
    PINMODE broche,mode (0 entrée, 1 sortie, 2 entrée avec résistance de tirage
                        interne : bouton vers GND, appuyé = 0)
    DWRITE broche,valeur (0 ou 1)      AWRITE broche,valeur (MLI de 0 à 255)
    DREAD(broche) : 0 ou 1             AREAD(entrée) : 0 à 1023 (entrée analogique 0 à 15
                                       sur le Mega, numéro de broche sur l'ESP32)

  Variables :
    A à Z      : nombres (environ 7 chiffres significatifs)
    A$ à Z$    : chaînes de caractères (BAS_STR_MAX caractères au maximum)
    A(n) à Z(n): tableaux de nombres, indices de 0 à n (10 par défaut sans DIM)

  Opérateurs :
    + - * / ^ (touche flèche haut) MOD     = <> < > <= >=     AND OR NOT
    + pour coller deux chaînes

  Fonctions :
    ABS INT SGN SQR SIN COS TAN ATN LOG EXP
    RND(n) : nombre entre 0 et 1 si n <= 1, entier de 1 à n sinon
    LEN(a$) VAL(a$) ASC(a$) CHR$(n) STR$(x) LEFT$(a$,n) RIGHT$(a$,n) MID$(a$,début[,n])
    INSTR([début,]a$,b$) : position de b$ dans a$ (0 si absente)
    HEX$(n) UPPER$(a$) LOWER$(a$)  (UPPER$ et LOWER$ ne changent que les lettres sans accent)
    INKEY$ : touche frappée (chaîne vide si aucune), sans attendre
    KEY : code de la touche frappée (0 si aucune), sans attendre. Il donne aussi les
          touches de fonction : ENVOI 4929, RETOUR 4930, REPETITION 4931, GUIDE 4932,
          CORRECTION 4935, SUITE 4936, et pour les autres touches le code (Unicode) du
          caractère : A 65, a 97, é 233... ANNULATION et SOMMAIRE arrêtent le programme.
    POINT(x,y) : -1 si le point est allumé, 0 sinon
    TIMER : millisecondes écoulées depuis le RUN
    TIME$ ("hh:mm:ss") DATE$ ("JJ/MM/AAAA") HOUR MINUTE SECOND DAY MONTH YEAR (horloge RTC)

  Erreurs :
    Le message donne le numéro de la ligne, puis réaffiche la ligne avec, en vidéo
    inverse, l'endroit où l'interpréteur s'est arrêté.

  Dessin :
    L'écran est une grille de 80 x 72 points (x de 0 à 79, y de 0 à 71, origine en haut
    à gauche), obtenue avec les caractères semi-graphiques : chaque case de texte contient
    2 x 3 points. PLOT allume un point, LINE trace un segment, BOX un rectangle et CIRCLE
    un cercle de rayon r (mesuré horizontalement : les points du Minitel sont plus larges
    que hauts, le cercle s'étend donc sur r x 1,25 points verticalement pour paraître rond).
    Avec c = 0, ils éteignent ; avec p = 1, BOX et CIRCLE sont pleins.
    La couleur est celle fixée par COLOR, le fond celui fixé par PAPER. CLS efface le dessin.
    Les points sont gardés en mémoire pour pouvoir en allumer plusieurs dans une même case.
    Cette mémoire ne suit pas le défilement de l'écran : mieux vaut dessiner après un CLS
    et placer le texte avec LOCATE. Un texte écrit sur le dessin est remplacé par les
    points de la case dès qu'on y dessine à nouveau.
    La case en bas à droite (points 78-79, 69-71) n'est pas affichée, car y écrire
    ferait défiler l'écran.
*/
//////////////////////////////////////////////////////////////////////////////////////

// Tailles (à ajuster selon la mémoire disponible)
#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
#define BAS_PROG_SIZE 16384  // Taille maximale du programme, en octets
#define BAS_STR_MAX   78     // Longueur maximale d'une chaîne
#define BAS_ARR_SIZE  1000   // Nombre total d'éléments de tableaux
#else
#define BAS_PROG_SIZE 2048   // Taille maximale du programme, en octets
#define BAS_STR_MAX   32     // Longueur maximale d'une chaîne
#define BAS_ARR_SIZE  64     // Nombre total d'éléments de tableaux
#endif
#define BAS_LINE_MAX  78     // Longueur maximale d'une ligne
#define BAS_TEXT_MAX  (BAS_LINE_MAX + 24)  // Longueur maximale d'une ligne rangée (RENUM peut l'allonger)
#define BAS_GOSUB_MAX 12     // Nombre maximal de GOSUB imbriqués
#define BAS_FOR_MAX   8      // Nombre maximal de FOR imbriqués
#define BAS_WHILE_MAX 6      // Nombre maximal de WHILE imbriqués
#define BAS_FILE_MAX  2      // Nombre de fichiers de données ouverts en même temps
#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
#define BAS_DEPTH_MAX 8      // Nombre maximal de parenthèses et de fonctions imbriquées
#else
#define BAS_DEPTH_MAX 6      // Moins sur l'ATmega : chaque niveau utilise de la pile (mémoire vive)
#endif
#define BAS_GFX_W     80     // Largeur du dessin, en points (40 cases x 2)
#define BAS_GFX_H     72     // Hauteur du dessin, en points (24 rangées x 3)

// Erreurs
#define BAS_E_SYNTAXE   1
#define BAS_E_DIVZERO   2
#define BAS_E_LIGNE     3
#define BAS_E_RETURN    4
#define BAS_E_NEXT      5
#define BAS_E_MEMOIRE   6
#define BAS_E_TYPE      7
#define BAS_E_INDICE    8
#define BAS_E_FICHIER   9
#define BAS_E_IMBRIC    10
#define BAS_E_VALEUR    11
#define BAS_E_BREAK     12
#define BAS_E_DATA      13
#define BAS_E_WEND      14
#define BAS_E_WHILE     15
#define BAS_E_NONOUVERT 16
#define BAS_E_FINFICH   17
#define BAS_E_INTROUV   18
#define BAS_E_DEJA      19
#define BAS_E_BROCHE    20
#define BAS_E_HORLOGE   21
#define BAS_E_LONGUE    22
#define BAS_E_PC        23

// Résultats de la saisie d'une ligne
#define BAS_K_OK      0
#define BAS_K_QUITTER 1
#define BAS_K_ANNULER 2
#define BAS_K_AIDE    3

// Le programme : une suite de lignes triées par numéro.
// Chaque ligne : numéro (2 octets), longueur du texte avec son zéro final (1 octet), texte, zéro.
byte basProg[BAS_PROG_SIZE];
unsigned int basProgLen = 0;

// Les variables
float basVar[26];                    // A à Z
char basStr[26][BAS_STR_MAX + 1];    // A$ à Z$
float basArr[BAS_ARR_SIZE];          // Eléments de tous les tableaux
int basArrBase[26];                  // Position du tableau dans basArr (-1 : non défini)
int basArrDim[26];                   // Dernier indice du tableau
int basArrUsed = 0;

// Les piles
byte* basGsLine[BAS_GOSUB_MAX];      // GOSUB : ligne de retour
char* basGsPos[BAS_GOSUB_MAX];       // GOSUB : position de retour dans la ligne
byte basGsTop = 0;
byte basForVar[BAS_FOR_MAX];         // FOR : variable
float basForLimit[BAS_FOR_MAX];      // FOR : valeur finale
float basForStep[BAS_FOR_MAX];       // FOR : pas
byte* basForLine[BAS_FOR_MAX];       // FOR : ligne de la boucle
char* basForPos[BAS_FOR_MAX];        // FOR : position dans la ligne
byte basForTop = 0;
byte* basWhLine[BAS_WHILE_MAX];      // WHILE : ligne de la boucle
char* basWhPos[BAS_WHILE_MAX];       // WHILE : position du mot WHILE
byte basWhTop = 0;

// DATA / READ
byte* basDataLine = NULL;            // Ligne où se trouve la prochaine donnée (NULL : début)
char* basDataPos = NULL;             // Position dans cette ligne
boolean basDataIn = false;           // Est-on dans la liste d'un DATA ?

// Fichiers de données
File basFile[BAS_FILE_MAX];

// Horloge (définie dans Horloge.ino)
extern boolean horlogeOK;

// L'exécution
char basLine[BAS_LINE_MAX + 2];      // Ligne tapée au clavier
char basInBuf[BAS_LINE_MAX + 2];     // Réponse à un INPUT, ligne lue dans un fichier
char* basP;                          // Position courante dans le texte exécuté
byte* basCur;                        // Ligne en cours d'exécution (NULL : ligne tapée au clavier)
byte basErr;                         // Erreur en cours (0 : aucune)
boolean basStop;                     // Demande d'arrêt de l'exécution
boolean basJumped;                   // Un saut vient d'avoir lieu
boolean basAttr;                     // Un attribut d'affichage a été changé (COLOR, PAPER...)
byte basInk = 7;                     // COLOR : couleur des caractères et des points
byte basPaper = 0;                   // PAPER : couleur de fond
byte basStyle = 0;                   // INVERSE, FLASH, UNDERLINE et SIZE (voir BAS_ST_...)
boolean basEditPending = false;      // EDIT : une ligne attend d'être corrigée (dans basInBuf)
byte basDepth;                       // Niveau d'imbrication des expressions
byte basCol = 0;                     // Colonne du curseur (0 à 39)
unsigned long basKey = 0;            // Touche frappée pendant l'exécution, pour INKEY$ et KEY
byte basGfx[BAS_GFX_W * BAS_GFX_H / 8];  // Le dessin : un bit par point (720 octets)
unsigned long basTimer0 = 0;         // Origine de TIMER (millis au moment du RUN)
unsigned int basErrLigne = 0;        // Numéro de la ligne introuvable (erreur BAS_E_LIGNE)

float basExpr();
void basStrExpr(char* dst);
void basStatement();

#define BAS_KW(mot) basKw(PSTR(mot))

// Bits de basStyle
#define BAS_ST_INVERSE  0x01
#define BAS_ST_FLASH    0x02
#define BAS_ST_SOULIGNE 0x04
#define BAS_ST_HAUT     0x10  // Double hauteur
#define BAS_ST_LARGE    0x20  // Double largeur (avec BAS_ST_HAUT : double taille)

// Les fonctions qui ont besoin d'un tampon de chaîne sont séparées et ne doivent pas
// être intégrées par le compilateur dans celles qui les appellent : ainsi leur tampon
// n'occupe la pile que lorsqu'elles sont réellement utilisées, et pas à chaque niveau
// d'imbrication des expressions.
#define BAS_NOINLINE __attribute__((noinline))

//////////////////////////////////////////////////////////////////////////////////////
// Affichage
//////////////////////////////////////////////////////////////////////////////////////

void basAvance()
{
  basCol += (basStyle & BAS_ST_LARGE) ? 2 : 1;  // En double largeur, un caractère occupe deux colonnes
  if (basCol >= 40) basCol -= 40;  // Le Minitel passe tout seul à la rangée suivante
}

//////////////////////////////////////////////////////////////////////////////////////

void basEsc(byte attribut)
{
  // Envoie un attribut au Minitel sans passer par minitel.attributs(), qui descend le
  // curseur d'une rangée pour la double hauteur
  minitel.writeByte(0x1B);
  minitel.writeByte(attribut);
}

//////////////////////////////////////////////////////////////////////////////////////

void basRestoreAttr()
{
  // Après un caractère semi-graphique, le Minitel a annulé la taille, l'inversion et le
  // soulignement (STUM 1B p. 91 et 94) : on les rétablit. La couleur, le fond et le
  // clignotement sont conservés.
  if (basStyle & BAS_ST_INVERSE) basEsc(INVERSION_FOND);
  if (basStyle & BAS_ST_SOULIGNE) basEsc(DEBUT_LIGNAGE);
  byte taille = (basStyle & (BAS_ST_HAUT | BAS_ST_LARGE)) >> 4;
  if (taille) basEsc(GRANDEUR_NORMALE + taille);
}

//////////////////////////////////////////////////////////////////////////////////////

void basResetAttr()
{
  // Revient à l'affichage normal : blanc sur fond noir, sans effet, taille normale
  minitel.attributs(CARACTERE_BLANC);
  minitel.attributs(FOND_NORMAL);
  minitel.attributs(FIXE);
  minitel.attributs(FIN_LIGNAGE);
  minitel.attributs(GRANDEUR_NORMALE);
  minitel.attributs(FOND_NOIR);
  basInk = 7;
  basPaper = 0;
  basStyle = 0;
}

//////////////////////////////////////////////////////////////////////////////////////

void basNewLine()
{
  minitel.writeByte(0x0D);  // Retour en début de rangée
  minitel.writeByte(0x0A);  // Rangée suivante (l'écran défile en mode rouleau)
  basCol = 0;
}

//////////////////////////////////////////////////////////////////////////////////////

void basPuts(const char* s)
{
  // Affiche une chaîne codée en UTF-8
  while (*s) {
    byte c = (byte) *s;
    if (c == '\n') {
      basNewLine();
      s++;
    }
    else if (c < 0x20) {  // Code de contrôle (CHR$) : envoyé tel quel au Minitel
      minitel.writeByte(c);
      if (c == 0x0D) basCol = 0;
      if (c == 0x08 && basCol > 0) basCol--;
      s++;
    }
    else if (c < 0x80) {
      minitel.writeByte(c);
      basAvance();
      s++;
    }
    else {  // Caractère sur plusieurs octets (lettre accentuée...)
      char seq[5];
      byte n = 0;
      seq[n++] = *s++;
      while ((((byte) *s) & 0xC0) == 0x80 && n < 4) seq[n++] = *s++;
      seq[n] = 0;
      if (c == 0xC2 || c == 0xC3 || c == 0xC5 || c == 0xCE || c == 0xE2) {
        minitel.print(seq);
      }
      else {
        minitel.writeByte('?');  // Caractère inconnu du Minitel
      }
      basAvance();
    }
  }
}

//////////////////////////////////////////////////////////////////////////////////////

void basPutsP(const char* p)
{
  // Affiche une chaîne rangée en mémoire programme (PSTR)
  char buf[64];
  strncpy_P(buf, p, 63);
  buf[63] = 0;
  basPuts(buf);
}

//////////////////////////////////////////////////////////////////////////////////////

char* basUtoa(unsigned long n, char* out)
{
  // Ecrit un entier en décimal. Renvoie la position du zéro final.
  char tmp[11];
  byte i = 0;
  do {
    tmp[i++] = '0' + (n % 10);
    n /= 10;
  } while (n > 0);
  while (i > 0) *out++ = tmp[--i];
  *out = 0;
  return out;
}

//////////////////////////////////////////////////////////////////////////////////////

void basNumToStr(float v, char* out)
{
  // Convertit un nombre en chaîne (out : 20 caractères au moins)
  if (v != v) { strcpy_P(out, PSTR("NAN")); return; }  // Pas un nombre
  if (v < 0) { *out++ = '-'; v = -v; }
  if (v > 3.4e38) { strcpy_P(out, PSTR("INF")); return; }  // Infini
  if (v == 0) { strcpy_P(out, PSTR("0")); return; }
  int expo = 0;
  boolean sci = (v >= 1e9 || v < 1e-4);
  if (sci) {  // Notation scientifique : on ramène le nombre entre 1 et 10
    while (v >= 10) { v /= 10; expo++; }
    while (v < 1) { v *= 10; expo--; }
  }
  unsigned long ip = (unsigned long) v;  // Partie entière
  float fp = v - ip;                     // Partie décimale
  byte chiffres = 0;
  for (unsigned long t = ip; t > 0; t /= 10) chiffres++;
  int dec = 7 - chiffres;                // 7 chiffres significatifs au total
  if (dec > 6) dec = 6;
  if (dec < 0) dec = 0;
  unsigned long echelle = 1;
  for (int i = 0; i < dec; i++) echelle *= 10;
  unsigned long f = (unsigned long) (fp * echelle + 0.5);
  if (f >= echelle) { ip++; f -= echelle; }
  if (sci && ip >= 10) { ip = 1; f = 0; expo++; }
  out = basUtoa(ip, out);
  if (f > 0) {
    char tmp[8];
    for (int i = dec - 1; i >= 0; i--) { tmp[i] = '0' + (f % 10); f /= 10; }
    while (dec > 0 && tmp[dec - 1] == '0') dec--;  // On enlève les zéros inutiles
    *out++ = '.';
    for (int i = 0; i < dec; i++) *out++ = tmp[i];
    *out = 0;
  }
  if (sci) {
    *out++ = 'E';
    if (expo < 0) { *out++ = '-'; expo = -expo; }
    basUtoa(expo, out);
  }
}

//////////////////////////////////////////////////////////////////////////////////////

void basPrintNum(float v)
{
  char buf[20];
  basNumToStr(v, buf);
  basPuts(buf);
}

//////////////////////////////////////////////////////////////////////////////////////

void basShowErrorLine(const char* texte, const char* pos)
{
  // Réaffiche la ligne fautive, avec en vidéo inverse le caractère où l'analyse s'est
  // arrêtée (une espace inverse si c'est la fin de la ligne).
  if (basCur != NULL) {
    basPrintNum(basLineNum(basCur));
    basPuts(" ");
  }
  char morceau[BAS_TEXT_MAX + 1];
  unsigned int n = pos - texte;
  if (n > BAS_TEXT_MAX) n = BAS_TEXT_MAX;
  memcpy(morceau, texte, n);
  morceau[n] = 0;
  basPuts(morceau);
  // Le caractère fautif (éventuellement sur plusieurs octets)
  byte len = 0;
  if (*pos == 0) morceau[len++] = ' ';
  else {
    morceau[len++] = pos[0];
    while (len < 4 && (((byte) pos[len]) & 0xC0) == 0x80) { morceau[len] = pos[len]; len++; }
  }
  morceau[len] = 0;
  minitel.attributs(INVERSION_FOND);
  basPuts(morceau);
  minitel.attributs(FOND_NORMAL);
  if (*pos != 0) basPuts(pos + len);
  if (basCol != 0) basNewLine();
}

//////////////////////////////////////////////////////////////////////////////////////

void basPrintError()
{
  const char* pos = basP;  // Endroit où l'analyse s'est arrêtée
  basResetAttr();  // Le message doit rester lisible après un COLOR, un SIZE...
  if (basCol != 0) basNewLine();
  switch (basErr) {
    case BAS_E_SYNTAXE : basPutsP(PSTR("Erreur de syntaxe")); break;
    case BAS_E_DIVZERO : basPutsP(PSTR("Division par zéro")); break;
    case BAS_E_LIGNE   :
      basPutsP(PSTR("Ligne "));
      if (basErrLigne > 0) { basPrintNum(basErrLigne); basPuts(" "); }
      basPutsP(PSTR("inexistante"));
      break;
    case BAS_E_RETURN  : basPutsP(PSTR("RETURN sans GOSUB")); break;
    case BAS_E_NEXT    : basPutsP(PSTR("NEXT sans FOR")); break;
    case BAS_E_MEMOIRE : basPutsP(PSTR("Mémoire pleine")); break;
    case BAS_E_TYPE    : basPutsP(PSTR("Type incompatible")); break;
    case BAS_E_INDICE  : basPutsP(PSTR("Indice hors limites")); break;
    case BAS_E_FICHIER : basPutsP(PSTR("Erreur de fichier")); break;
    case BAS_E_IMBRIC  : basPutsP(PSTR("Trop d'imbrications")); break;
    case BAS_E_VALEUR  : basPutsP(PSTR("Valeur incorrecte")); break;
    case BAS_E_BREAK   : basPutsP(PSTR("Interruption")); break;
    case BAS_E_DATA    : basPutsP(PSTR("Plus de DATA")); break;
    case BAS_E_WEND    : basPutsP(PSTR("WEND sans WHILE")); break;
    case BAS_E_WHILE   : basPutsP(PSTR("WHILE sans WEND")); break;
    case BAS_E_NONOUVERT : basPutsP(PSTR("Fichier non ouvert")); break;
    case BAS_E_FINFICH : basPutsP(PSTR("Fin de fichier")); break;
    case BAS_E_INTROUV : basPutsP(PSTR("Fichier introuvable")); break;
    case BAS_E_DEJA    : basPutsP(PSTR("Fichier déjà ouvert")); break;
    case BAS_E_BROCHE  : basPutsP(PSTR("Broche interdite")); break;
    case BAS_E_HORLOGE : basPutsP(PSTR("Horloge absente")); break;
    case BAS_E_LONGUE  : basPutsP(PSTR("Ligne trop longue")); break;
    case BAS_E_PC      : basPutsP(PSTR("PC absent")); break;
  }
  if (basCur != NULL) {
    basPutsP(PSTR(" en ligne "));
    basPrintNum(basLineNum(basCur));
  }
  basNewLine();
  if (basErr != BAS_E_BREAK) {
    // La ligne fautive : celle du programme, ou la ligne tapée au clavier
    const char* texte = NULL;
    if (basCur != NULL) texte = (const char*) (basCur + 3);
    else if (pos >= basLine && pos < basLine + sizeof(basLine)) texte = basLine;
    if (texte != NULL && pos >= texte && pos <= texte + strlen(texte)) basShowErrorLine(texte, pos);
    minitel.bip();
  }
  basErrLigne = 0;
}

//////////////////////////////////////////////////////////////////////////////////////

void basKeyName(const char* nom)
{
  // Affiche le nom d'une touche (rangé en mémoire programme) en vidéo inverse
  minitel.attributs(INVERSION_FOND);
  basPutsP(nom);
  minitel.attributs(FOND_NORMAL);
}

//////////////////////////////////////////////////////////////////////////////////////

#define BAS_HELP_PAGES 4     // Nombre de pages de l'aide

void basHelpPage(byte page)
{
  // Affiche une page de l'aide (de 0 à BAS_HELP_PAGES - 1) sur un écran effacé
  minitel.newScreen();
  basCol = 0;
  minitel.attributs(INVERSION_FOND);
  basPutsP(PSTR(" reBASIC - Aide "));
  basPrintNum(page + 1);
  basPuts("/");
  basPrintNum(BAS_HELP_PAGES);
  basPuts(" ");
  minitel.attributs(FOND_NORMAL);
  basNewLine();
  basNewLine();
  if (page == 0) {
    basPutsP(PSTR("Commandes :\n"));
    basPutsP(PSTR(" RUN LIST [n][-m] [/P] NEW MEM AIDE\n"));
    basPutsP(PSTR(" EDIT n  RENUM [début][,pas]\n"));
    basPutsP(PSTR(" SAVE \"NOM\" LOAD \"NOM\" MERGE \"NOM\"\n"));
    basPutsP(PSTR(" FILES KILL \"NOM\"\n"));
    basPutsP(PSTR(" \"PC:NOM\" FILES \"PC:\" : sur le PC\n"));
    basNewLine();
    basPutsP(PSTR("Instructions :\n"));
    basPutsP(PSTR(" PRINT (ou ?) avec ; , TAB(n)\n"));
    basPutsP(PSTR(" INPUT [\"texte\";] v[,v...]\n"));
    basPutsP(PSTR(" [LET] v = expression\n"));
    basPutsP(PSTR(" IF cond THEN ... [ELSE ...]\n"));
    basPutsP(PSTR(" GOTO n  GOSUB n  RETURN\n"));
    basPutsP(PSTR(" ON x GOTO n1,n2...  ON x GOSUB ...\n"));
    basPutsP(PSTR(" FOR v=a TO b [STEP c] ... NEXT [v]\n"));
    basPutsP(PSTR(" WHILE cond ... WEND\n"));
    basPutsP(PSTR(" DATA  READ v[,v...]  RESTORE [n]\n"));
    basPutsP(PSTR(" DIM A(n)  REM  END  STOP\n"));
  }
  else if (page == 1) {
    basPutsP(PSTR("Ecran (couleurs 0 à 7) :\n"));
    basPutsP(PSTR(" CLS  LOCATE col,rang\n"));
    basPutsP(PSTR(" COLOR n  PAPER n (fond)\n"));
    basPutsP(PSTR(" INVERSE n  FLASH n  UNDERLINE n\n"));
    basPutsP(PSTR(" SIZE n (0 normale, 1 double hauteur,\n"));
    basPutsP(PSTR("  2 double largeur, 3 double taille)\n"));
    basPutsP(PSTR(" PAPER et UNDERLINE commencent au\n"));
    basPutsP(PSTR(" premier espace qui suit.\n"));
    basNewLine();
    basPutsP(PSTR("Son :\n"));
    basPutsP(PSTR(" BEEP  SOUND f,ms  PAUSE ms\n"));
    basNewLine();
    basPutsP(PSTR("Dessin (80 x 72 points) :\n"));
    basPutsP(PSTR(" PLOT x,y[,c]\n"));
    basPutsP(PSTR(" LINE x1,y1,x2,y2[,c]\n"));
    basPutsP(PSTR(" BOX x1,y1,x2,y2[,c[,p]]\n"));
    basPutsP(PSTR(" CIRCLE x,y,r[,c[,p]]\n"));
    basPutsP(PSTR(" (c=0 efface, p=1 plein)  POINT(x,y)\n"));
  }
  else if (page == 2) {
    basPutsP(PSTR("Fonctions :\n"));
    basPutsP(PSTR(" ABS INT SGN SQR SIN COS TAN ATN\n"));
    basPutsP(PSTR(" LOG EXP RND(n) POINT(x,y)\n"));
    basPutsP(PSTR(" LEN VAL ASC CHR$ STR$ HEX$\n"));
    basPutsP(PSTR(" LEFT$ RIGHT$ MID$ INSTR\n"));
    basPutsP(PSTR(" UPPER$ LOWER$ INKEY$ KEY\n"));
    basPutsP(PSTR(" TIMER (ms depuis le RUN)\n"));
    basPutsP(PSTR(" TIME$ DATE$ HOUR MINUTE SECOND\n"));
    basPutsP(PSTR(" DAY MONTH YEAR\n"));
    basNewLine();
    basPutsP(PSTR("Opérateurs : + - * / ^ MOD\n"));
    basPutsP(PSTR(" = <> < > <= >= AND OR NOT\n"));
    basNewLine();
    basPutsP(PSTR("Codes de KEY : ENVOI 4929\n"));
    basPutsP(PSTR(" RETOUR 4930  REPETITION 4931\n"));
    basPutsP(PSTR(" GUIDE 4932  CORRECTION 4935\n"));
    basPutsP(PSTR(" SUITE 4936  autres : code du caractère\n"));
  }
  else {
    basPutsP(PSTR("Fichiers de données (.TXT) :\n"));
    basPutsP(PSTR(" OPEN \"NOM\" FOR INPUT AS #1\n"));
    basPutsP(PSTR("  (ou OUTPUT, APPEND ; #1 ou #2)\n"));
    basPutsP(PSTR(" PRINT #1,...  INPUT #1,v[,v...]\n"));
    basPutsP(PSTR(" LINE INPUT #1,A$  EOF(1)\n"));
    basPutsP(PSTR(" CLOSE [#1]\n"));
    basNewLine();
    basPutsP(PSTR("Broches :\n"));
    basPutsP(PSTR(" PINMODE b,m (0 entrée, 1 sortie,\n"));
    basPutsP(PSTR("  2 entrée avec résistance de tirage\n"));
    basPutsP(PSTR("  interne, bouton vers GND : appuyé=0)\n"));
    basPutsP(PSTR(" DWRITE b,v  AWRITE b,v (0 à 255)\n"));
    basPutsP(PSTR(" DREAD(b)  AREAD(a)\n"));
    basNewLine();
    basPutsP(PSTR("Touches :\n "));
    basKeyName(PSTR("ENVOI"));
    basPutsP(PSTR(" valide  "));
    basKeyName(PSTR("CORRECTION"));
    basPutsP(PSTR(" efface\n "));
    basKeyName(PSTR("ANNULATION"));
    basPutsP(PSTR(" interrompt\n "));
    basKeyName(PSTR("GUIDE"));
    basPutsP(PSTR(" aide  "));
    basKeyName(PSTR("SOMMAIRE"));
    basPutsP(PSTR(" menu\n"));
  }
  // Le bas de l'écran : les touches de l'aide, selon la page
  // ("RETOUR - SUITE - ENVOI : reBASIC", "RETOUR - ...", "SUITE - ...")
  minitel.moveCursorXY(1, 24);
  basCol = 0;
  if (page > 0) basKeyName(PSTR("RETOUR"));
  if (page > 0 && page < BAS_HELP_PAGES - 1) basPuts(" - ");
  if (page < BAS_HELP_PAGES - 1) basKeyName(PSTR("SUITE"));
  basPuts(" - ");
  basKeyName(PSTR("ENVOI"));
  basPutsP(PSTR(" : reBASIC"));
}

//////////////////////////////////////////////////////////////////////////////////////

void basHelp()
{
  // L'aide, sur plusieurs pages : SUITE et RETOUR pour passer d'une page à l'autre,
  // ENVOI (et elle seule) pour revenir au BASIC sur un écran effacé.
  byte page = 0;
  basHelpPage(page);
  while (true) {
    unsigned long k = minitel.getKeyCode();
    if (k == 0) continue;
    if (k == SUITE) {
      if (page < BAS_HELP_PAGES - 1) basHelpPage(++page);
      else minitel.bip();
    }
    else if (k == RETOUR) {
      if (page > 0) basHelpPage(--page);
      else minitel.bip();
    }
    else if (k == ENVOI || k == 0x0D) break;
  }
  minitel.newScreen();  // Le dessin et le texte précédents ont disparu
  basCol = 0;
  basGfxClear();
}

//////
// Clavier
//////////////////////////////////////////////////////////////////////////////////////

void basKeyToStr(unsigned long k, char* s)
{
  // Convertit le code d'une touche en chaîne UTF-8 (s : 5 caractères au moins).
  // La chaîne est vide si la touche ne correspond pas à un caractère.
  s[0] = 0;
  if (k == 0x2191) k = '^';  // La flèche vers le haut sert d'opérateur puissance
  if (k >= 0x20 && k < 0x7F) {
    s[0] = (char) k;
    s[1] = 0;
  }
  else if (k >= 0x80 && k <= 0xFFFF) {
    String u = minitel.getString(k);
    if (u.length() > 0 && u.length() <= 4) strcpy(s, u.c_str());
  }
}

//////////////////////////////////////////////////////////////////////////////////////

void basErase()
{
  // Efface à l'écran le caractère situé à gauche du curseur
  byte largeur = (basStyle & BAS_ST_LARGE) ? 2 : 1;
  if (basCol < largeur) {  // Le caractère est à la fin de la rangée précédente
    minitel.moveCursorUp(1);
    minitel.moveCursorRight(40 - largeur);
    basCol = 40 - largeur;
  }
  else {
    minitel.moveCursorLeft(largeur);
    basCol -= largeur;
  }
  minitel.clearLineFromCursor();
}

//////////////////////////////////////////////////////////////////////////////////////

byte basReadLine(char* buf, byte maxLen, boolean garder)
{
  // Saisie d'une ligne au clavier. Renvoie BAS_K_OK, BAS_K_QUITTER, BAS_K_ANNULER ou BAS_K_AIDE.
  // Si garder est vrai, buf contient déjà un texte (EDIT) : il est affiché et peut être
  // corrigé avec CORRECTION ou complété.
  byte n = 0;
  byte resultat = 0xFF;
  if (garder) {
    n = strlen(buf);
    basPuts(buf);
  }
  else {
    buf[0] = 0;
  }
  minitel.cursor();
  while (resultat == 0xFF) {
    unsigned long k = minitel.getKeyCode();
    if (k == 0) continue;
    if (k == ENVOI || k == 0x0D) resultat = BAS_K_OK;
    else if (k == SOMMAIRE) resultat = BAS_K_QUITTER;
    else if (k == ANNULATION) resultat = BAS_K_ANNULER;
    else if (k == GUIDE) resultat = BAS_K_AIDE;
    else if (k == CORRECTION) {
      if (n > 0) {
        while (n > 0 && (((byte) buf[n - 1]) & 0xC0) == 0x80) n--;  // Caractère sur plusieurs octets
        if (n > 0) n--;
        buf[n] = 0;
        basErase();
      }
    }
    else {
      char s[5];
      basKeyToStr(k, s);
      byte len = strlen(s);
      if (len > 0) {
        if (n + len <= maxLen) {
          memcpy(buf + n, s, len);
          n += len;
          buf[n] = 0;
          basPuts(s);
        }
        else {
          minitel.bip();  // La ligne est pleine
        }
      }
    }
  }
  minitel.noCursor();
  return resultat;
}

//////////////////////////////////////////////////////////////////////////////////////

void basCheckBreak()
{
  // Regarde si SOMMAIRE ou ANNULATION a été frappée pendant l'exécution.
  // Une autre touche est gardée pour INKEY$ et KEY.
  unsigned long k = minitel.getKeyCode();
  if (k == 0) return;
  if (k == SOMMAIRE || k == ANNULATION) basErr = BAS_E_BREAK;
  else basKey = k;
}

//////////////////////////////////////////////////////////////////////////////////////

void basWait(unsigned long duree)
{
  // Attente en millisecondes, que l'on peut interrompre
  unsigned long debut = millis();
  while (millis() - debut < duree && basErr == 0) {
    basCheckBreak();
  }
}

//////////////////////////////////////////////////////////////////////////////////////
// Dessin (points semi-graphiques)
//////////////////////////////////////////////////////////////////////////////////////

boolean basGfxGet(int x, int y)
{
  // Le point est-il allumé ? (faux en dehors de l'écran)
  if (x < 0 || x >= BAS_GFX_W || y < 0 || y >= BAS_GFX_H) return false;
  unsigned int i = (unsigned int) y * BAS_GFX_W + x;
  return (basGfx[i >> 3] >> (i & 7)) & 1;
}

//////////////////////////////////////////////////////////////////////////////////////

void basGfxSet(int x, int y, boolean allume)
{
  // Allume ou éteint un point en mémoire (sans effet en dehors de l'écran)
  if (x < 0 || x >= BAS_GFX_W || y < 0 || y >= BAS_GFX_H) return;
  unsigned int i = (unsigned int) y * BAS_GFX_W + x;
  if (allume) basGfx[i >> 3] |= (1 << (i & 7));
  else basGfx[i >> 3] &= ~(1 << (i & 7));
}

//////////////////////////////////////////////////////////////////////////////////////

byte basGfxPoints(int cx, int cy)
{
  // Les 6 points de la case (cx, cy), du coin supérieur gauche (bit 5) au coin inférieur droit (bit 0)
  int x = cx * 2;
  int y = cy * 3;
  byte points = 0;
  if (basGfxGet(x, y))         points |= 0b100000;
  if (basGfxGet(x + 1, y))     points |= 0b010000;
  if (basGfxGet(x, y + 1))     points |= 0b001000;
  if (basGfxGet(x + 1, y + 1)) points |= 0b000100;
  if (basGfxGet(x, y + 2))     points |= 0b000010;
  if (basGfxGet(x + 1, y + 2)) points |= 0b000001;
  return points;
}

//////////////////////////////////////////////////////////////////////////////////////

void basGfxCell(int cx, int cy)
{
  // Affiche la case (cx de 0 à 39, cy de 0 à 23) avec ses 6 points
  if (cx < 0 || cx > 39 || cy < 0 || cy > 23) return;
  if (cx == 39 && cy == 23) return;  // Ecrire dans la dernière case ferait défiler l'écran
  minitel.moveCursorXY(cx + 1, cy + 1);
  minitel.graphicMode();
  minitel.graphic(basGfxPoints(cx, cy));
  minitel.textMode();
  basRestoreAttr();
  basCol = (cx + 1) % 40;
}

//////////////////////////////////////////////////////////////////////////////////////

void basGfxMark(byte* cases, long x, long y, boolean allume)
{
  // Allume ou éteint un point et note sa case dans cases (960 bits : une par case de l'écran)
  if (x < 0 || x >= BAS_GFX_W || y < 0 || y >= BAS_GFX_H) return;
  basGfxSet((int) x, (int) y, allume);
  unsigned int k = (unsigned int) (y / 3) * 40 + (unsigned int) (x / 2);
  cases[k >> 3] |= (1 << (k & 7));
}

//////////////////////////////////////////////////////////////////////////////////////

void basGfxFlush(byte* cases)
{
  // Affiche les cases notées par basGfxMark, une seule fois chacune. Des cases qui se
  // suivent sur une rangée sont envoyées à la suite, sans déplacer le curseur.
  boolean graphique = false;
  int suivante = -1;  // Case où se trouve le curseur
  for (int k = 0; k < 960; k++) {
    if (!(cases[k >> 3] & (1 << (k & 7)))) continue;
    int cx = k % 40;
    int cy = k / 40;
    if (cx == 39 && cy == 23) continue;  // Ecrire dans la dernière case ferait défiler l'écran
    if (k != suivante) minitel.moveCursorXY(cx + 1, cy + 1);
    if (!graphique) {
      minitel.graphicMode();
      graphique = true;
    }
    minitel.graphic(basGfxPoints(cx, cy));
    suivante = (cx == 39) ? -1 : k + 1;
    basCol = (cx + 1) % 40;
  }
  if (graphique) {
    minitel.textMode();
    basRestoreAttr();
  }
}

//////////////////////////////////////////////////////////////////////////////////////

void basGfxBox(long x1, long y1, long x2, long y2, boolean allume, boolean plein)
{
  // BOX : rectangle de coins (x1, y1) et (x2, y2), plein ou non
  byte cases[120];
  memset(cases, 0, sizeof(cases));
  if (x1 > x2) { long t = x1; x1 = x2; x2 = t; }
  if (y1 > y2) { long t = y1; y1 = y2; y2 = t; }
  long xa = (x1 < 0) ? 0 : x1;  // Limites dans l'écran, pour ne pas boucler pour rien
  long xb = (x2 >= BAS_GFX_W) ? BAS_GFX_W - 1 : x2;
  long ya = (y1 < 0) ? 0 : y1;
  long yb = (y2 >= BAS_GFX_H) ? BAS_GFX_H - 1 : y2;
  if (plein) {
    for (long y = ya; y <= yb; y++) {
      for (long x = xa; x <= xb; x++) basGfxMark(cases, x, y, allume);
    }
  }
  else {
    for (long x = xa; x <= xb; x++) {
      basGfxMark(cases, x, y1, allume);
      basGfxMark(cases, x, y2, allume);
    }
    for (long y = ya; y <= yb; y++) {
      basGfxMark(cases, x1, y, allume);
      basGfxMark(cases, x2, y, allume);
    }
  }
  basGfxFlush(cases);
}

//////////////////////////////////////////////////////////////////////////////////////

void basGfxLine(long x1, long y1, long x2, long y2, boolean allume)
{
  // Trace un segment (algorithme de Bresenham). Chaque case touchée est affichée une fois.
  byte cases[120];
  memset(cases, 0, sizeof(cases));
  long dx = (x2 > x1) ? x2 - x1 : x1 - x2;
  long dy = (y2 > y1) ? y1 - y2 : y2 - y1;  // Négatif
  long sx = (x1 < x2) ? 1 : -1;
  long sy = (y1 < y2) ? 1 : -1;
  long ecart = dx + dy;
  while (true) {
    basGfxMark(cases, x1, y1, allume);
    if (x1 == x2 && y1 == y2) break;
    long e2 = 2 * ecart;
    if (e2 >= dy) { ecart += dy; x1 += sx; }
    if (e2 <= dx) { ecart += dx; y1 += sy; }
  }
  basGfxFlush(cases);
}

//////////////////////////////////////////////////////////////////////////////////////

long basCircleHeight(long dx, long r)
{
  // Demi-hauteur, en points, de la colonne dx d'un cercle de rayon r (rayon vertical
  // r x 1,25). Les rayons sont augmentés d'un demi-point pour un contour sans pointes.
  float rx = r + 0.5;
  float ry = r * 1.25 + 0.5;
  float f = 1.0 - ((float) dx * dx) / (rx * rx);
  if (f <= 0) return 0;
  return (long) (ry * sqrt(f));
}

//////////////////////////////////////////////////////////////////////////////////////

void basGfxCircle(long xc, long yc, long r, boolean allume, boolean plein)
{
  // CIRCLE : cercle de centre (xc, yc) et de rayon r, plein ou non. Un point du Minitel
  // est 1,25 fois plus large que haut : le rayon vertical vaut r x 1,25 pour que le
  // cercle paraisse rond.
  byte cases[120];
  memset(cases, 0, sizeof(cases));
  for (long dx = 0; dx <= r; dx++) {
    long haut = basCircleHeight(dx, r);
    // Pour le contour, chaque colonne descend jusqu'à la hauteur de sa voisine extérieure :
    // le tracé n'a pas de trou, même là où il est presque vertical
    long bas = 0;
    if (!plein && dx < r) {
      bas = basCircleHeight(dx + 1, r) + 1;
      if (bas > haut) bas = haut;
    }
    if (yc - haut < 0 && yc + haut >= BAS_GFX_H && plein) {  // Colonne plus haute que l'écran
      bas = 0;
    }
    for (long dy = bas; dy <= haut; dy++) {
      if (yc + dy >= 0 && yc - dy < BAS_GFX_H) {
        basGfxMark(cases, xc + dx, yc + dy, allume);
        basGfxMark(cases, xc - dx, yc + dy, allume);
        basGfxMark(cases, xc + dx, yc - dy, allume);
        basGfxMark(cases, xc - dx, yc - dy, allume);
      }
    }
  }
  basGfxFlush(cases);
}

//////

void basGfxClear()
{
  memset(basGfx, 0, sizeof(basGfx));
}

//////////////////////////////////////////////////////////////////////////////////////
// Analyse du texte
//////////////////////////////////////////////////////////////////////////////////////

void basSkip()
{
  while (*basP == ' ') basP++;
}

//////////////////////////////////////////////////////////////////////////////////////

boolean basIsLetter(char c)
{
  return (c >= 'A' && c <= 'Z');
}

//////////////////////////////////////////////////////////////////////////////////////

boolean basIsDigit(char c)
{
  return (c >= '0' && c <= '9');
}

//////////////////////////////////////////////////////////////////////////////////////

boolean basKw(const char* mot)
{
  // Teste la présence d'un mot-clé (rangé en mémoire programme) à la position courante.
  // S'il est présent, on avance après lui.
  basSkip();
  byte n = strlen_P(mot);
  if (strncmp_P(basP, mot, n) != 0) return false;
  char dernier = (char) pgm_read_byte(mot + n - 1);
  if (basIsLetter(dernier) && basIsLetter(basP[n])) return false;  // Le mot continue : ce n'est pas lui
  basP += n;
  return true;
}

//////////////////////////////////////////////////////////////////////////////////////

void basExpect(char c)
{
  basSkip();
  if (*basP == c) basP++;
  else if (basErr == 0) basErr = BAS_E_SYNTAXE;
}

//////////////////////////////////////////////////////////////////////////////////////

char* basNextWord(char* p)
{
  // Cherche, à partir de p, le prochain mot (suite de lettres) en dehors des chaînes.
  // Renvoie NULL à la fin de la ligne ou sur un commentaire (REM ou ').
  while (*p) {
    if (*p == '"') {
      p++;
      while (*p && *p != '"') p++;
      if (*p) p++;
    }
    else if (*p == '\'') return NULL;
    else if (basIsLetter(*p)) {
      if (strncmp_P(p, PSTR("REM"), 3) == 0) return NULL;
      return p;
    }
    else p++;
  }
  return NULL;
}

//////////////////////////////////////////////////////////////////////////////////////

char* basWordEnd(char* p)
{
  // Renvoie la position qui suit le mot commençant en p
  while (basIsLetter(*p)) p++;
  return p;
}

//////////////////////////////////////////////////////////////////////////////////////

boolean basWordIs(const char* p, const char* mot)
{
  // Le mot qui commence en p commence-t-il par le mot-clé mot (rangé en mémoire programme) ?
  return strncmp_P(p, mot, strlen_P(mot)) == 0;
}

//////////////////////////////////////////////////////////////////////////////////////

boolean basAtElse()
{
  // La position courante est-elle sur un ELSE ? (fin de la branche THEN)
  basSkip();
  return basWordIs(basP, PSTR("ELSE"));
}

//////////////////////////////////////////////////////////////////////////////////////

char* basFindElse(char* p)
{
  // Cherche le ELSE qui correspond au IF en cours (les IF imbriqués sur la même
  // ligne ont chacun le leur). Renvoie la position qui suit ce ELSE, ou NULL.
  byte niveau = 0;
  while ((p = basNextWord(p)) != NULL) {
    if (basWordIs(p, PSTR("IF"))) niveau++;
    else if (basWordIs(p, PSTR("ELSE"))) {
      if (niveau == 0) return p + 4;
      niveau--;
    }
    p = basWordEnd(p);
  }
  return NULL;
}

//////////////////////////////////////////////////////////////////////////////////////

char* basSkipStatement(char* p)
{
  // Avance jusqu'au ':' qui termine l'instruction (hors chaînes) ou jusqu'à la fin de la ligne
  while (*p && *p != ':') {
    if (*p == '"') {
      p++;
      while (*p && *p != '"') p++;
      if (*p) p++;
    }
    else p++;
  }
  return p;
}

//////////////////////////////////////////////////////////////////////////////////////

void basUpper(char* s)
{
  // Convertit en majuscules ce qui n'est pas entre guillemets
  boolean guillemets = false;
  for (; *s; s++) {
    if (*s == '"') guillemets = !guillemets;
    else if (!guillemets && *s >= 'a' && *s <= 'z') *s -= 32;
  }
}

//////////////////////////////////////////////////////////////////////////////////////

boolean basParseNum(char** pp, float* resultat)
{
  // Lit un nombre (sans signe) à la position *pp. Renvoie false s'il n'y en a pas.
  char* p = *pp;
  float v = 0;
  boolean chiffre = false;
  while (basIsDigit(*p)) { v = v * 10 + (*p - '0'); p++; chiffre = true; }
  if (*p == '.') {
    p++;
    float poids = 0.1;
    while (basIsDigit(*p)) { v += (*p - '0') * poids; poids /= 10; p++; chiffre = true; }
  }
  if (!chiffre) return false;
  if (*p == 'E' && (basIsDigit(p[1]) || ((p[1] == '+' || p[1] == '-') && basIsDigit(p[2])))) {
    p++;
    boolean negatif = false;
    if (*p == '+') p++;
    else if (*p == '-') { negatif = true; p++; }
    int e = 0;
    while (basIsDigit(*p)) { if (e < 100) e = e * 10 + (*p - '0'); p++; }
    while (e-- > 0) { if (negatif) v /= 10; else v *= 10; }
  }
  *pp = p;
  *resultat = v;
  return true;
}

//////////////////////////////////////////////////////////////////////////////////////

boolean basIsStrStart()
{
  // Y a-t-il une chaîne à la position courante ? (guillemet, A$, CHR$(...)
  basSkip();
  const char* q = basP;
  if (*q == '"') return true;
  if (!basIsLetter(*q)) return false;
  while (basIsLetter(*q)) q++;
  return (*q == '$');
}

//////////////////////////////////////////////////////////////////////////////////////

byte basRelOp()
{
  // Lit un opérateur de comparaison : 1 =   2 <>   3 <   4 >   5 <=   6 >=   (0 : aucun)
  basSkip();
  if (*basP == '=') { basP++; return 1; }
  if (*basP == '<') {
    basP++;
    if (*basP == '>') { basP++; return 2; }
    if (*basP == '=') { basP++; return 5; }
    return 3;
  }
  if (*basP == '>') {
    basP++;
    if (*basP == '=') { basP++; return 6; }
    return 4;
  }
  return 0;
}

//////////////////////////////////////////////////////////////////////////////////////

float basCompare(byte op, float d)
{
  // d : différence entre les deux termes comparés. Vrai vaut -1, faux vaut 0.
  boolean vrai = false;
  switch (op) {
    case 1 : vrai = (d == 0); break;
    case 2 : vrai = (d != 0); break;
    case 3 : vrai = (d < 0); break;
    case 4 : vrai = (d > 0); break;
    case 5 : vrai = (d <= 0); break;
    case 6 : vrai = (d >= 0); break;
  }
  return vrai ? -1 : 0;
}

//////////////////////////////////////////////////////////////////////////////////////
// Variables et tableaux
//////////////////////////////////////////////////////////////////////////////////////

void basClearVars()
{
  for (byte i = 0; i < 26; i++) {
    basVar[i] = 0;
    basStr[i][0] = 0;
    basArrBase[i] = -1;
    basArrDim[i] = 0;
  }
  basArrUsed = 0;
  basResetStacks();
}

//////////////////////////////////////////////////////////////////////////////////////

void basResetStacks()
{
  // Oublie les GOSUB, FOR et WHILE en cours et repart du premier DATA
  // (à faire dès que le programme change : ces piles pointent dans le programme)
  basGsTop = 0;
  basForTop = 0;
  basWhTop = 0;
  basDataLine = NULL;
  basDataPos = NULL;
  basDataIn = false;
}

//////////////////////////////////////////////////////////////////////////////////////

void basDim(byte idx, long n)
{
  // Réserve le tableau idx avec les indices 0 à n
  if (basArrBase[idx] >= 0 || n < 0) { basErr = BAS_E_VALEUR; return; }
  if (n + 1 > BAS_ARR_SIZE - basArrUsed) { basErr = BAS_E_MEMOIRE; return; }
  basArrBase[idx] = basArrUsed;
  basArrDim[idx] = (int) n;
  for (long i = 0; i <= n; i++) basArr[basArrUsed + i] = 0;
  basArrUsed += (int) n + 1;
}

//////////////////////////////////////////////////////////////////////////////////////

float* basArrRef(byte idx)
{
  // Lit "(indice)" et renvoie l'adresse de l'élément du tableau idx (NULL en cas d'erreur)
  basExpect('(');
  long i = (long) basExpr();
  basExpect(')');
  if (basErr) return NULL;
  if (basArrBase[idx] < 0) {  // Sans DIM, le tableau a les indices 0 à 10
    basDim(idx, 10);
    if (basErr) return NULL;
  }
  if (i < 0 || i > basArrDim[idx]) { basErr = BAS_E_INDICE; return NULL; }
  return &basArr[basArrBase[idx] + i];
}

//////////////////////////////////////////////////////////////////////////////////////
// Chaînes de caractères
//////////////////////////////////////////////////////////////////////////////////////

int basU8Len(const char* s)
{
  // Nombre de caractères d'une chaîne UTF-8
  int n = 0;
  for (; *s; s++) {
    if ((((byte) *s) & 0xC0) != 0x80) n++;
  }
  return n;
}

//////////////////////////////////////////////////////////////////////////////////////

const char* basU8Skip(const char* s, long n)
{
  // Avance de n caractères dans une chaîne UTF-8
  while (n > 0 && *s) {
    s++;
    while ((((byte) *s) & 0xC0) == 0x80) s++;
    n--;
  }
  return s;
}

//////////////////////////////////////////////////////////////////////////////////////

void basStrAppend(char* dst, const char* src, const char* fin)
{
  // Ajoute à dst les octets de src jusqu'à fin (exclu), sans dépasser BAS_STR_MAX
  // et sans couper un caractère sur plusieurs octets.
  byte n = strlen(dst);
  while (src < fin && *src) {
    byte len = 1;
    while ((((byte) src[len]) & 0xC0) == 0x80) len++;
    if (n + len > BAS_STR_MAX) break;
    while (len-- > 0) dst[n++] = *src++;
  }
  dst[n] = 0;
}

//////////////////////////////////////////////////////////////////////////////////////

boolean basEnter()
{
  // Entrée dans un niveau d'imbrication (parenthèse, fonction). Renvoie false s'il y en a trop.
  if (basDepth >= BAS_DEPTH_MAX) {
    if (basErr == 0) basErr = BAS_E_IMBRIC;
    return false;
  }
  basDepth++;
  return true;
}

//////////////////////////////////////////////////////////////////////////////////////

BAS_NOINLINE void basStrNum(char* dst)
{
  // STR$(x)
  basExpect('(');
  float v = basExpr();
  basExpect(')');
  if (basErr) return;
  char buf[20];
  basNumToStr(v, buf);
  basStrAppend(dst, buf, buf + strlen(buf));
}

//////////////////////////////////////////////////////////////////////////////////////

BAS_NOINLINE void basStrPart(char* dst, char genre)
{
  // LEFT$ (genre 'L'), RIGHT$ ('R') ou MID$ ('M')
  char src[BAS_STR_MAX + 1];
  basExpect('(');
  basStrExpr(src);
  basExpect(',');
  long a = (long) basExpr();
  long b = BAS_STR_MAX;
  basSkip();
  if (genre == 'M' && *basP == ',') {
    basP++;
    b = (long) basExpr();
  }
  basExpect(')');
  if (basErr) return;
  if (a < 0 || b < 0 || (genre == 'M' && a < 1)) { basErr = BAS_E_VALEUR; return; }
  const char* debut = src;
  const char* fin;
  if (genre == 'L') fin = basU8Skip(src, a);
  else if (genre == 'R') {
    long total = basU8Len(src);
    debut = basU8Skip(src, (a < total) ? total - a : 0);
    fin = src + strlen(src);
  }
  else {
    debut = basU8Skip(src, a - 1);
    fin = basU8Skip(debut, b);
  }
  basStrAppend(dst, debut, fin);
}

//////////////////////////////////////////////////////////////////////////////////////

BAS_NOINLINE void basStrCase(char* dst, boolean majuscules)
{
  // UPPER$ (majuscules) ou LOWER$ : seules les lettres sans accent changent
  char src[BAS_STR_MAX + 1];
  basExpect('(');
  basStrExpr(src);
  basExpect(')');
  if (basErr) return;
  for (char* q = src; *q; q++) {
    if (majuscules && *q >= 'a' && *q <= 'z') *q -= 32;
    else if (!majuscules && *q >= 'A' && *q <= 'Z') *q += 32;
  }
  basStrAppend(dst, src, src + strlen(src));
}

//////////////////////////////////////////////////////////////////////////////////////

void basHexStr(char* dst)
{
  // HEX$(n) : écriture hexadécimale (les nombres négatifs sur 32 bits)
  float v = basArg();
  if (basErr) return;
  if (v < -2147483648.0 || v > 4294967295.0) { basErr = BAS_E_VALEUR; return; }
  uint32_t n = (v < 0) ? (uint32_t) (int32_t) v : (uint32_t) v;
  char tmp[9];
  byte i = 0;
  do {
    byte c = n & 0x0F;
    tmp[i++] = (c < 10) ? '0' + c : 'A' + c - 10;
    n >>= 4;
  } while (n > 0);
  byte k = 0;
  while (i > 0) dst[k++] = tmp[--i];
  dst[k] = 0;
}

//////////////////////////////////////////////////////////////////////////////////////

void bas2Digits(char* s, int n)
{
  s[0] = '0' + (n / 10) % 10;
  s[1] = '0' + n % 10;
}

//////////////////////////////////////////////////////////////////////////////////////

void basClockStr(char* dst, boolean date)
{
  // DATE$ ("JJ/MM/AAAA") ou TIME$ ("hh:mm:ss")
  if (!horlogeOK && !horlogeAttendrePC(500)) { basErr = BAS_E_HORLOGE; return; }
  DateTime t = horlogeMaintenant();
  if (date) {
    bas2Digits(dst, t.day());
    dst[2] = '/';
    bas2Digits(dst + 3, t.month());
    dst[5] = '/';
    bas2Digits(dst + 6, t.year() / 100);
    bas2Digits(dst + 8, t.year());
    dst[10] = 0;
  }
  else {
    bas2Digits(dst, t.hour());
    dst[2] = ':';
    bas2Digits(dst + 3, t.minute());
    dst[5] = ':';
    bas2Digits(dst + 6, t.second());
    dst[8] = 0;
  }
}

//////////////////////////////////////////////////////////////////////////////////////

void basStrPrimary(char* dst)
{
  // Lit un élément de chaîne et le range dans dst (BAS_STR_MAX + 1 octets)
  dst[0] = 0;
  basSkip();
  if (*basP == '"') {  // Chaîne entre guillemets
    basP++;
    char* debut = basP;
    while (*basP && *basP != '"') basP++;
    basStrAppend(dst, debut, basP);
    if (*basP == '"') basP++;
    return;
  }
  if (basIsLetter(basP[0]) && basP[1] == '$') {  // Variable A$ à Z$
    strcpy(dst, basStr[basP[0] - 'A']);
    basP += 2;
    return;
  }
  if (BAS_KW("INKEY$")) {
    if (basKey == 0) basKey = minitel.getKeyCode();
    unsigned long k = basKey;
    basKey = 0;
    if (k == SOMMAIRE || k == ANNULATION) { basErr = BAS_E_BREAK; return; }
    if (k == ENVOI || k == 0x0D) { dst[0] = 13; dst[1] = 0; return; }
    basKeyToStr(k, dst);
    return;
  }
  if (BAS_KW("DATE$")) { basClockStr(dst, true); return; }
  if (BAS_KW("TIME$")) { basClockStr(dst, false); return; }
  // Les fonctions avec des paramètres
  char genre = 0;
  if (BAS_KW("CHR$")) genre = 'C';
  else if (BAS_KW("STR$")) genre = 'S';
  else if (BAS_KW("LEFT$")) genre = 'L';
  else if (BAS_KW("RIGHT$")) genre = 'R';
  else if (BAS_KW("MID$")) genre = 'M';
  else if (BAS_KW("HEX$")) genre = 'H';
  else if (BAS_KW("UPPER$")) genre = 'U';
  else if (BAS_KW("LOWER$")) genre = 'W';
  if (genre == 0) {
    basErr = BAS_E_TYPE;
    return;
  }
  if (!basEnter()) return;
  if (genre == 'C') {
    basExpect('(');
    long n = (long) basExpr();
    basExpect(')');
    if (basErr == 0) {
      if (n < 1 || n > 127) basErr = BAS_E_VALEUR;
      else { dst[0] = (char) n; dst[1] = 0; }
    }
  }
  else if (genre == 'S') basStrNum(dst);
  else if (genre == 'H') basHexStr(dst);
  else if (genre == 'U' || genre == 'W') basStrCase(dst, genre == 'U');
  else basStrPart(dst, genre);
  basDepth--;
}

//////////////////////////////////////////////////////////////////////////////////////

BAS_NOINLINE void basStrConcat(char* dst)
{
  // Ajoute à dst l'élément de chaîne qui suit un +
  char suite[BAS_STR_MAX + 1];
  basStrPrimary(suite);
  basStrAppend(dst, suite, suite + strlen(suite));
}

//////////////////////////////////////////////////////////////////////////////////////

void basStrExpr(char* dst)
{
  // Expression de type chaîne : éléments collés avec +
  basStrPrimary(dst);
  while (basErr == 0) {
    basSkip();
    if (*basP != '+') break;
    basP++;
    basStrConcat(dst);
  }
}

//////////////////////////////////////////////////////////////////////////////////////
// Expressions numériques
//////////////////////////////////////////////////////////////////////////////////////

float basArg()
{
  // Lit "(expression)"
  basExpect('(');
  float v = basExpr();
  basExpect(')');
  return v;
}

//////////////////////////////////////////////////////////////////////////////////////

BAS_NOINLINE float basStrCompare()
{
  // Comparaison de deux chaînes : A$ = "OUI", A$ < B$...
  char a[BAS_STR_MAX + 1];
  char b[BAS_STR_MAX + 1];
  basStrExpr(a);
  if (basErr) return 0;
  byte op = basRelOp();
  if (op == 0) { basErr = BAS_E_TYPE; return 0; }
  basStrExpr(b);
  if (basErr) return 0;
  return basCompare(op, (float) strcmp(a, b));
}

//////////////////////////////////////////////////////////////////////////////////////

BAS_NOINLINE float basStrFunc(char quoi)
{
  // LEN (quoi 'N'), VAL ('L') ou ASC ('C')
  char s[BAS_STR_MAX + 1];
  float v = 0;
  basExpect('(');
  basStrExpr(s);
  basExpect(')');
  if (basErr) return 0;
  if (quoi == 'N') v = basU8Len(s);
  else if (quoi == 'C') v = (byte) s[0];
  else v = basStrToNum(s);
  return v;
}

//////////////////////////////////////////////////////////////////////////////////////

float basStrToNum(char* s)
{
  // Valeur du nombre écrit au début de s (0 s'il n'y en a pas), comme VAL
  float v = 0;
  char* q = s;
  while (*q == ' ') q++;
  boolean negatif = (*q == '-');
  if (*q == '-' || *q == '+') q++;
  if (!basParseNum(&q, &v)) v = 0;
  return negatif ? -v : v;
}

//////////////////////////////////////////////////////////////////////////////////////

BAS_NOINLINE float basInstr()
{
  // INSTR([début,]a$,b$) : position (en caractères) de b$ dans a$ à partir de début, 0 si absente
  char a[BAS_STR_MAX + 1];
  char b[BAS_STR_MAX + 1];
  long debut = 1;
  basExpect('(');
  if (!basIsStrStart()) {
    debut = (long) basExpr();
    basExpect(',');
  }
  basStrExpr(a);
  basExpect(',');
  basStrExpr(b);
  basExpect(')');
  if (basErr) return 0;
  if (debut < 1) { basErr = BAS_E_VALEUR; return 0; }
  if (debut > basU8Len(a) + 1) return 0;
  const char* depart = basU8Skip(a, debut - 1);
  const char* trouve = strstr(depart, b);
  if (trouve == NULL) return 0;
  return debut + basU8Len(depart) - basU8Len(trouve);
}

//////////////////////////////////////////////////////////////////////////////////////

float basClockNum(byte quoi)
{
  // HOUR (quoi 'H'), MINUTE ('N'), SECOND ('S'), DAY ('D'), MONTH ('M') ou YEAR ('Y')
  // Sans horloge RTC ni heure connue, on la demande au PC (passerelle)
  if (!horlogeOK && !horlogeAttendrePC(500)) { basErr = BAS_E_HORLOGE; return 0; }
  DateTime t = horlogeMaintenant();
  switch (quoi) {
    case 'H' : return t.hour();
    case 'N' : return t.minute();
    case 'S' : return t.second();
    case 'D' : return t.day();
    case 'M' : return t.month();
  }
  return t.year();
}

//////////////////////////////////////////////////////////////////////////////////////

boolean basPinOK(long broche)
{
  // La broche peut-elle être utilisée par un programme BASIC ?
  // Celles du Minitel, de la carte SD, de l'horloge et du buzzer sont refusées.
#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
  if (broche < 0 || broche > 39) return false;
  if (broche >= 6 && broche <= 11) return false;   // Mémoire flash
  if (broche == 16 || broche == 17) return false;  // Serial2 (Minitel)
  if (broche == 21 || broche == 22) return false;  // I2C (horloge)
  if (broche == 5 || broche == 18 || broche == 19 || broche == 23) return false;  // SPI (carte SD)
#else
  if (broche < 0 || broche >= NUM_DIGITAL_PINS) return false;
  if (broche <= 1) return false;                     // Serial (PC)
  if (broche >= 16 && broche <= 21) return false;    // Serial2, Serial1 (Minitel), I2C (horloge)
  if (broche == 34 || broche == 36) return false;    // SQW et 32K de l'horloge
  if (broche >= 50 && broche <= 53) return false;    // SPI (carte SD)
#endif
  if (broche == BUZZER || broche == CHIP_SELECT) return false;
  return true;
}

//////////////////////////////////////////////////////////////////////////////////////

float basPinRead(boolean analogique)
{
  // DREAD(broche) ou AREAD(entrée)
  long n = (long) basArg();
  if (basErr) return 0;
#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
  if (!basPinOK(n)) { basErr = BAS_E_BROCHE; return 0; }
  return analogique ? analogRead((int) n) : digitalRead((int) n);
#else
  if (analogique) {
    if (n < 0 || n > 15) { basErr = BAS_E_BROCHE; return 0; }  // Entrées analogiques A0 à A15
    return analogRead((int) n);
  }
  if (!basPinOK(n)) { basErr = BAS_E_BROCHE; return 0; }
  return digitalRead((int) n);
#endif
}

//////////////////////////////////////////////////////////////////////////////////////

float basPrimary()
{
  basSkip();
  char c = *basP;

  // Comparaison de deux chaînes
  if (basIsStrStart()) {
    if (!basEnter()) return 0;
    float v = basStrCompare();
    basDepth--;
    return v;
  }

  // Nombre
  if (basIsDigit(c) || c == '.') {
    float v = 0;
    if (!basParseNum(&basP, &v)) basErr = BAS_E_SYNTAXE;
    return v;
  }

  // Parenthèses
  if (c == '(') {
    if (!basEnter()) return 0;
    float v = basArg();
    basDepth--;
    return v;
  }

  // Fonctions
  if (basIsLetter(c) && basIsLetter(basP[1])) {
    if (!basEnter()) return 0;
    float v = 0;
    if (BAS_KW("LEN") || BAS_KW("VAL") || BAS_KW("ASC")) {
      v = basStrFunc(basP[-1]);  // N, L ou C : dernière lettre du nom de la fonction
    }
    else if (BAS_KW("ABS")) { v = basArg(); if (v < 0) v = -v; }
    else if (BAS_KW("INT")) { v = floor(basArg()); }
    else if (BAS_KW("SGN")) { v = basArg(); v = (v > 0) ? 1 : ((v < 0) ? -1 : 0); }
    else if (BAS_KW("SQR")) { v = basArg(); if (v < 0) basErr = BAS_E_VALEUR; else v = sqrt(v); }
    else if (BAS_KW("SIN")) { v = sin(basArg()); }
    else if (BAS_KW("COS")) { v = cos(basArg()); }
    else if (BAS_KW("TAN")) { v = tan(basArg()); }
    else if (BAS_KW("ATN")) { v = atan(basArg()); }
    else if (BAS_KW("LOG")) { v = basArg(); if (v <= 0) basErr = BAS_E_VALEUR; else v = log(v); }
    else if (BAS_KW("EXP")) { v = exp(basArg()); }
    else if (BAS_KW("POINT")) {
      basExpect('(');
      long x = (long) basExpr();
      basExpect(',');
      long y = (long) basExpr();
      basExpect(')');
      if (basErr == 0 && x >= 0 && x < BAS_GFX_W && y >= 0 && y < BAS_GFX_H) {
        v = basGfxGet((int) x, (int) y) ? -1 : 0;
      }
    }
    else if (BAS_KW("INSTR")) v = basInstr();
    else if (BAS_KW("KEY")) {
      // Code de la touche frappée, comme le donne Minitel1B_Hard (0 si aucune) : les
      // touches de fonction (SUITE 4936...) n'ont pas de caractère, INKEY$ ne les voit pas
      if (basKey == 0) basKey = minitel.getKeyCode();
      unsigned long k = basKey;
      basKey = 0;
      if (k == SOMMAIRE || k == ANNULATION) basErr = BAS_E_BREAK;
      else v = (float) k;
    }
    else if (BAS_KW("TIMER")) v = (float) (millis() - basTimer0);
    else if (BAS_KW("HOUR")) v = basClockNum('H');
    else if (BAS_KW("MINUTE")) v = basClockNum('N');
    else if (BAS_KW("SECOND")) v = basClockNum('S');
    else if (BAS_KW("DAY")) v = basClockNum('D');
    else if (BAS_KW("MONTH")) v = basClockNum('M');
    else if (BAS_KW("YEAR")) v = basClockNum('Y');
    else if (BAS_KW("DREAD")) v = basPinRead(false);
    else if (BAS_KW("AREAD")) v = basPinRead(true);
    else if (BAS_KW("EOF")) {
      long n = (long) basArg();
      if (basErr == 0) {
        if (n < 1 || n > BAS_FILE_MAX) basErr = BAS_E_VALEUR;
        else if (!basFile[n - 1]) basErr = BAS_E_NONOUVERT;
        else v = basFile[n - 1].available() ? 0 : -1;
      }
    }
    else if (BAS_KW("RND")) {
      v = basArg();
      if (v <= 1) v = random(0, 10000) / 10000.0;   // Nombre de 0 à 0,9999
      else v = random(1, (long) v + 1);             // Entier de 1 à n
    }
    else basErr = BAS_E_SYNTAXE;
    basDepth--;
    return v;
  }

  // Variable ou élément de tableau
  if (basIsLetter(c)) {
    byte idx = c - 'A';
    basP++;
    if (*basP == '(') {
      if (!basEnter()) return 0;
      float* element = basArrRef(idx);
      basDepth--;
      return (element != NULL) ? *element : 0;
    }
    return basVar[idx];
  }

  basErr = BAS_E_SYNTAXE;
  return 0;
}

//////////////////////////////////////////////////////////////////////////////////////

float basUnary();

float basPower()
{
  float v = basPrimary();
  basSkip();
  if (basErr == 0 && *basP == '^') {
    basP++;
    v = pow(v, basUnary());
  }
  return v;
}

//////////////////////////////////////////////////////////////////////////////////////

float basUnary()
{
  basSkip();
  if (*basP == '-') { basP++; return -basUnary(); }
  if (*basP == '+') { basP++; return basUnary(); }
  return basPower();
}

//////////////////////////////////////////////////////////////////////////////////////

float basMul()
{
  float v = basUnary();
  while (basErr == 0) {
    basSkip();
    if (*basP == '*') { basP++; v *= basUnary(); }
    else if (*basP == '/') {
      basP++;
      float d = basUnary();
      if (d == 0) { if (basErr == 0) basErr = BAS_E_DIVZERO; }
      else v /= d;
    }
    else if (BAS_KW("MOD")) {
      long d = (long) basUnary();
      if (d == 0) { if (basErr == 0) basErr = BAS_E_DIVZERO; }
      else v = (long) v % d;
    }
    else break;
  }
  return v;
}

//////////////////////////////////////////////////////////////////////////////////////

float basAdd()
{
  float v = basMul();
  while (basErr == 0) {
    basSkip();
    if (*basP == '+') { basP++; v += basMul(); }
    else if (*basP == '-') { basP++; v -= basMul(); }
    else break;
  }
  return v;
}

//////////////////////////////////////////////////////////////////////////////////////

float basRel()
{
  float v = basAdd();
  if (basErr) return v;
  byte op = basRelOp();
  if (op != 0) v = basCompare(op, v - basAdd());
  return v;
}

//////////////////////////////////////////////////////////////////////////////////////

float basNot()
{
  if (BAS_KW("NOT")) return (float) (~((long) basNot()));
  return basRel();
}

//////////////////////////////////////////////////////////////////////////////////////

float basAnd()
{
  float v = basNot();
  while (basErr == 0 && BAS_KW("AND")) {
    long d = (long) basNot();
    v = (float) ((long) v & d);
  }
  return v;
}

//////////////////////////////////////////////////////////////////////////////////////

float basExpr()
{
  float v = basAnd();
  while (basErr == 0 && BAS_KW("OR")) {
    long d = (long) basAnd();
    v = (float) ((long) v | d);
  }
  return v;
}

//////////////////////////////////////////////////////////////////////////////////////
// Le programme en mémoire
//////////////////////////////////////////////////////////////////////////////////////

unsigned int basLineNum(byte* ligne)
{
  return (unsigned int) ligne[0] | ((unsigned int) ligne[1] << 8);
}

//////////////////////////////////////////////////////////////////////////////////////

byte* basProgEnd()
{
  return basProg + basProgLen;
}

//////////////////////////////////////////////////////////////////////////////////////

byte* basFindLine(unsigned int num)
{
  // Renvoie la première ligne dont le numéro est supérieur ou égal à num
  byte* p = basProg;
  while (p < basProgEnd() && basLineNum(p) < num) p += 3 + p[2];
  return p;
}

//////////////////////////////////////////////////////////////////////////////////////

boolean basStoreLine(unsigned int num, const char* texte)
{
  // Range, remplace ou supprime (texte vide) une ligne. Renvoie false si la mémoire est pleine.
  byte* p = basFindLine(num);
  unsigned int ancienne = 0;
  if (p < basProgEnd() && basLineNum(p) == num) ancienne = 3 + p[2];
  unsigned int len = strlen(texte);
  while (len > 0 && texte[len - 1] == ' ') len--;  // Sans les espaces de fin
  unsigned int nouvelle = (len > 0) ? 3 + len + 1 : 0;
  if (basProgLen - ancienne + nouvelle > BAS_PROG_SIZE) return false;
  basDataLine = NULL;  // Le programme change : on repart du premier DATA
  basDataIn = false;
  if (ancienne > 0) {
    memmove(p, p + ancienne, basProgEnd() - (p + ancienne));
    basProgLen -= ancienne;
  }
  if (nouvelle > 0) {
    memmove(p + nouvelle, p, basProgEnd() - p);
    p[0] = num & 0xFF;
    p[1] = num >> 8;
    p[2] = len + 1;
    memcpy(p + 3, texte, len);
    p[3 + len] = 0;
    basProgLen += nouvelle;
  }
  return true;
}

//////////////////////////////////////////////////////////////////////////////////////

void basEnterLine(char* texte)
{
  // Traite une ligne qui commence par un numéro (tapée au clavier ou lue dans un fichier)
  char* p = texte;
  unsigned long num = 0;
  while (basIsDigit(*p)) {
    if (num < 100000) num = num * 10 + (*p - '0');
    p++;
  }
  if (num < 1 || num > 65535) { basErr = BAS_E_SYNTAXE; return; }
  while (*p == ' ') p++;
  if (!basStoreLine((unsigned int) num, p)) basErr = BAS_E_MEMOIRE;
}

//////////////////////////////////////////////////////////////////////////////////////

void basGoto(unsigned int num)
{
  byte* p = basFindLine(num);
  if (p >= basProgEnd() || basLineNum(p) != num) { basErr = BAS_E_LIGNE; basErrLigne = num; return; }
  basCur = p;
  basP = (char*) (p + 3);
  basJumped = true;
}

//////////////////////////////////////////////////////////////////////////////////////

unsigned int basLineArg()
{
  // Lit un numéro de ligne (expression)
  float v = basExpr();
  if (basErr) return 0;
  if (v < 1 || v > 65535) { basErr = BAS_E_LIGNE; return 0; }
  return (unsigned int) v;
}

//////////////////////////////////////////////////////////////////////////////////////
// Fichiers sur la carte SD
//////////////////////////////////////////////////////////////////////////////////////

boolean basFileName(char* nom, const char* extension, boolean* pc)
{
  // Lit un nom de fichier ("JEU" ou "JEU.TXT") et construit le nom complet ("/JEU.BAS")
  // dans nom (14 octets). Sans extension, on ajoute celle qui est donnée (".BAS", ".TXT").
  // Si pc n'est pas NULL, le préfixe "PC:" ("PC:JEU") est accepté : *pc dit s'il y était
  // (le fichier est alors sur l'ordinateur, voir basPcSave). Sinon, "PC:" est refusé.
  char s[BAS_STR_MAX + 1];
  basStrExpr(s);
  if (basErr) return false;
  char* t = s;
  if (pc != NULL) {
    *pc = ((s[0] == 'P' || s[0] == 'p') && (s[1] == 'C' || s[1] == 'c') && s[2] == ':');
    if (*pc) t += 3;
  }
  int n = strlen(t);
  int point = -1;  // Position du point (-1 : pas de point)
  for (int i = 0; i < n; i++) {
    if (t[i] == '.') {
      if (point >= 0) { basErr = BAS_E_VALEUR; return false; }  // Deux points
      point = i;
    }
  }
  int base = (point < 0) ? n : point;         // Longueur du nom sans l'extension
  int ext = (point < 0) ? 0 : n - point - 1;  // Longueur de l'extension
  if (base < 1 || base > 8 || (point >= 0 && (ext < 1 || ext > 3))) {
    basErr = BAS_E_VALEUR;
    return false;
  }
  nom[0] = '/';
  for (int i = 0; i < n; i++) {
    char c = t[i];
    if (c >= 'a' && c <= 'z') c -= 32;
    if (!basIsLetter(c) && !basIsDigit(c) && c != '_' && c != '-' && i != point) {
      basErr = BAS_E_VALEUR;
      return false;
    }
    nom[1 + i] = c;
  }
  if (point < 0) strcpy_P(nom + 1 + n, extension);
  else nom[1 + n] = 0;
  return true;
}

//////////////////////////////////////////////////////////////////////////////////////

void basSave()
{
  char nom[14];
  boolean pc;
  if (!basFileName(nom, PSTR(".BAS"), &pc)) return;
  if (pc) { basPcSave(nom); return; }
  if (SD.exists(nom)) SD.remove(nom);
  File fichier = SD.open(nom, FILE_WRITE);
  if (!fichier) { basErr = BAS_E_FICHIER; return; }
  for (byte* p = basProg; p < basProgEnd(); p += 3 + p[2]) {
    fichier.print(basLineNum(p));
    fichier.print(' ');
    fichier.println((char*) (p + 3));
  }
  fichier.close();
}

//////////////////////////////////////////////////////////////////////////////////////

void basLoadStart(boolean fusion)
{
  // Avant de lire les lignes d'un LOAD (fusion faux) ou d'un MERGE (fusion vrai)
  basStop = true;  // Le programme change : on arrête l'exécution en cours
  if (!fusion) {
    basCloseAll();
    basProgLen = 0;
    basClearVars();
  }
  else {
    basResetStacks();
  }
}

//////////////////////////////////////////////////////////////////////////////////////

void basLoadLine(char* ligne)
{
  // Range une ligne lue dans un fichier (les lignes sans numéro sont ignorées)
  basUpper(ligne);
  char* p = ligne;
  while (*p == ' ') p++;
  if (basIsDigit(*p)) basEnterLine(p);
}

//////////////////////////////////////////////////////////////////////////////////////

void basLoad(boolean fusion)
{
  // LOAD (fusion faux) remplace le programme ; MERGE (fusion vrai) ajoute les lignes
  // du fichier au programme en cours (une ligne de même numéro est remplacée).
  char nom[14];
  boolean pc;
  if (!basFileName(nom, PSTR(".BAS"), &pc)) return;
  if (pc) { basPcLoad(nom, fusion); return; }
  if (!SD.exists(nom)) { basErr = BAS_E_INTROUV; return; }
  File fichier = SD.open(nom);
  if (!fichier) { basErr = BAS_E_FICHIER; return; }
  basLoadStart(fusion);
  byte n = 0;
  boolean fin = false;
  while (!fin && basErr == 0) {
    int c = fichier.available() ? fichier.read() : -1;
    if (c < 0) fin = true;
    if (c == '\r') continue;
    if (c == '\n' || c < 0) {  // Fin de ligne ou fin de fichier
      basInBuf[n] = 0;
      n = 0;
      basLoadLine(basInBuf);
    }
    else if (n < BAS_LINE_MAX) {
      basInBuf[n++] = (char) c;
    }
  }
  fichier.close();
}

//////////////////////////////////////////////////////////////////////////////////////
// Fichiers sur le PC : SAVE "PC:NOM", LOAD "PC:NOM", MERGE "PC:NOM", FILES "PC:"
//////////////////////////////////////////////////////////////////////////////////////
/*
  Les programmes sont rangés dans un dossier de l'ordinateur, relié à la carte par son
  câble USB (Serial, la liaison avec le PC). Sur l'ordinateur, le programme
  outils/passerelle/passerelle.py répond aux demandes de reBASIC.

  Chaque message est une ligne de texte terminée par un saut de ligne. Les lignes du
  dialogue commencent par @ (les lignes de programme commencent par leur numéro).
  L'heure (@HEURE, voir Horloge.ino) passe par le même câble :

    SAVE  : -> @SAVE NOM.BAS     <- @OK
            -> les lignes du programme, puis @FIN     <- @OK
    LOAD  : -> @LOAD NOM.BAS     <- @OK (ou @ERR INTROUVABLE)
            puis, pour chaque ligne : -> @SUITE   <- la ligne, ou @FIN à la fin
    FILES : -> @FILES            <- @OK
            puis, pour chaque fichier : -> @SUITE   <- "NOM.EXT taille", ou @FIN

  reBASIC demande les lignes une par une (@SUITE) : la mémoire de réception de
  l'Arduino Mega (64 octets) ne peut pas déborder. Les lignes qui ne commencent pas par @
  en réponse à une demande sont ignorées (messages de démarrage, par exemple).
  Sans réponse au bout de BAS_PC_DELAI millisecondes : erreur « PC absent ».
*/

#define BAS_PC_DELAI 3000

void basPcSendP(const char* p)
{
  // Envoie au PC un texte rangé en mémoire programme
  char c;
  while ((c = pgm_read_byte(p++)) != 0) Serial.write(c);
}

//////////////////////////////////////////////////////////////////////////////////////

void basPcSend(const char* s)
{
  // Envoie au PC un texte rangé en mémoire vive
  while (*s) Serial.write(*s++);
}

//////////////////////////////////////////////////////////////////////////////////////

boolean basPcRead(char* buf, byte max)
{
  // Lit une ligne envoyée par le PC, sans sa fin de ligne. Les caractères au-delà de max
  // sont ignorés. Faux, avec l'erreur « PC absent », si la ligne n'arrive pas à temps.
  byte n = 0;
  unsigned long debut = millis();
  while (millis() - debut < BAS_PC_DELAI) {
    if (!Serial.available()) continue;
    int c = Serial.read();
    if (c < 0 || c == '\r') continue;
    if (c == '\n') {
      buf[n] = 0;
      return true;
    }
    if (n < max) buf[n++] = (char) c;
  }
  basErr = BAS_E_PC;
  return false;
}

//////////////////////////////////////////////////////////////////////////////////////

boolean basPcAnswer()
{
  // Attend la réponse du PC à une demande : @OK (vrai), ou @ERR... (faux, avec une erreur)
  while (basPcRead(basInBuf, BAS_LINE_MAX)) {
    if (basInBuf[0] != '@') continue;  // Pas une réponse : on l'ignore
    if (horlogeDecoderPC(basInBuf)) continue;  // L'heure, demandée par l'écran d'accueil
    if (strcmp_P(basInBuf, PSTR("@OK")) == 0) return true;
    if (strcmp_P(basInBuf, PSTR("@ERR INTROUVABLE")) == 0) basErr = BAS_E_INTROUV;
    else basErr = BAS_E_FICHIER;
    return false;
  }
  return false;
}

//////////////////////////////////////////////////////////////////////////////////////

boolean basPcBegin(const char* demande, const char* nom)
{
  // Envoie une demande (@SAVE, @LOAD, @FILES), suivie du nom du fichier s'il y en a un
  // (sans la barre oblique du début), et attend que le PC l'accepte.
  while (Serial.available()) Serial.read();  // On oublie ce qui a pu arriver avant
  basPcSendP(demande);
  if (nom != NULL) {
    Serial.write(' ');
    basPcSend(nom + 1);
  }
  Serial.write('\n');
  return basPcAnswer();
}

//////////////////////////////////////////////////////////////////////////////////////

boolean basPcNext()
{
  // Demande la ligne suivante au PC (dans basInBuf). Faux à la fin (@FIN) ou en cas d'erreur.
  basPcSendP(PSTR("@SUITE\n"));
  if (!basPcRead(basInBuf, BAS_LINE_MAX)) return false;
  return strcmp_P(basInBuf, PSTR("@FIN")) != 0;
}

//////////////////////////////////////////////////////////////////////////////////////

void basPcSave(const char* nom)
{
  if (!basPcBegin(PSTR("@SAVE"), nom)) return;
  char num[6];
  for (byte* p = basProg; p < basProgEnd(); p += 3 + p[2]) {
    basUtoa(basLineNum(p), num);
    basPcSend(num);
    Serial.write(' ');
    basPcSend((char*) (p + 3));
    Serial.write('\n');
  }
  basPcSendP(PSTR("@FIN\n"));
  basPcAnswer();  // Le PC répond quand le fichier est écrit
}

//////////////////////////////////////////////////////////////////////////////////////

void basPcLoad(const char* nom, boolean fusion)
{
  if (!basPcBegin(PSTR("@LOAD"), nom)) return;
  basLoadStart(fusion);
  while (basErr == 0 && basPcNext()) basLoadLine(basInBuf);
}

//////////////////////////////////////////////////////////////////////////////////////

void basPcFiles()
{
  // Liste les fichiers du dossier du PC (le PC les envoie par ordre alphabétique)
  if (!basPcBegin(PSTR("@FILES"), NULL)) return;
  while (basErr == 0 && basPcNext()) {
    char* taille = strchr(basInBuf, ' ');  // "NOM.EXT taille"
    if (taille != NULL) *taille++ = 0;
    byte n = strlen(basInBuf);
    if (n > 4 && strcasecmp_P(basInBuf + n - 4, PSTR(".BAS")) == 0) basInBuf[n - 4] = 0;
    basPuts(basInBuf);
    if (taille != NULL) {
      while (basCol < 13) { minitel.writeByte(' '); basAvance(); }
      basPuts(taille);
    }
    basNewLine();
    basCheckBreak();
  }
}

//////////////////////////////////////////////////////////////////////////////////////

void basFiles()
{
  // Liste les fichiers de la carte SD par ordre alphabétique (les programmes .BAS sans
  // leur extension). Pour ne pas garder tous les noms en mémoire, chaque passage relit le
  // dossier et cherche le plus petit nom qui vient après le dernier nom affiché.
#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
  const byte MAX = 64;  // Noms longs possibles sur l'ESP32
#else
  const byte MAX = 13;  // Noms 8.3 sur l'ATmega (12 caractères) : peu de pile utilisée
#endif
  if (basIsStrStart()) {  // FILES "PC:" : les fichiers du PC
    char s[BAS_STR_MAX + 1];
    basStrExpr(s);
    if (basErr) return;
    if (strcasecmp_P(s, PSTR("PC:")) != 0 && strcasecmp_P(s, PSTR("PC")) != 0) { basErr = BAS_E_VALEUR; return; }
    basPcFiles();
    return;
  }
  char dernier[MAX], suivant[MAX], courant[MAX];
  dernier[0] = 0;
  bool premier = true;
  File racine = SD.open("/");
  if (!racine) { basErr = BAS_E_FICHIER; return; }
  while (basErr == 0) {
    bool trouve = false;
    unsigned long taille = 0;
    racine.rewindDirectory();
    while (true) {
      File f = racine.openNextFile();
      if (!f) break;
      if (!f.isDirectory()) {
        const char* complet = f.name();
        if (complet[0] == '/') complet++;
        // Les comparaisons se font sur le nom tronqué : un nom trop long ne peut pas
        // revenir sans fin (il serait toujours « après » sa version tronquée)
        strncpy(courant, complet, MAX - 1);
        courant[MAX - 1] = 0;
        if ((premier || strcmp(courant, dernier) > 0) && (!trouve || strcmp(courant, suivant) < 0)) {
          strcpy(suivant, courant);
          taille = f.size();
          trouve = true;
        }
      }
      f.close();
    }
    if (!trouve) break;
    char nom[14];
    strncpy(nom, suivant, 13);
    nom[13] = 0;
    byte n = strlen(nom);
    if (n > 4 && strcasecmp_P(nom + n - 4, PSTR(".BAS")) == 0) nom[n - 4] = 0;
    basPuts(nom);
    while (basCol < 13) { minitel.writeByte(' '); basAvance(); }
    basPrintNum(taille);
    basNewLine();
    strcpy(dernier, suivant);
    premier = false;
    basCheckBreak();
  }
  racine.close();
}

//////////////////////////////////////////////////////////////////////////////////////

void basCloseAll()
{
  for (byte i = 0; i < BAS_FILE_MAX; i++) {
    if (basFile[i]) basFile[i].close();
  }
}

//////////////////////////////////////////////////////////////////////////////////////

byte basChannel(boolean ouvert)
{
  // Lit un numéro de fichier (#1 ou 1) et renvoie son indice (0xFF en cas d'erreur).
  // Si ouvert est vrai, le fichier doit être ouvert.
  basSkip();
  if (*basP == '#') basP++;
  long n = (long) basExpr();
  if (basErr) return 0xFF;
  if (n < 1 || n > BAS_FILE_MAX) { basErr = BAS_E_VALEUR; return 0xFF; }
  if (ouvert && !basFile[n - 1]) { basErr = BAS_E_NONOUVERT; return 0xFF; }
  return (byte) (n - 1);
}

//////////////////////////////////////////////////////////////////////////////////////

void basOpen()
{
  // OPEN "NOM" FOR INPUT|OUTPUT|APPEND AS #n
  char nom[14];
  if (!basFileName(nom, PSTR(".TXT"), NULL)) return;
  if (!BAS_KW("FOR")) { basErr = BAS_E_SYNTAXE; return; }
  char mode;
  if (BAS_KW("INPUT")) mode = 'I';
  else if (BAS_KW("OUTPUT")) mode = 'O';
  else if (BAS_KW("APPEND")) mode = 'A';
  else { basErr = BAS_E_SYNTAXE; return; }
  if (!BAS_KW("AS")) { basErr = BAS_E_SYNTAXE; return; }
  byte i = basChannel(false);
  if (basErr) return;
  if (basFile[i]) { basErr = BAS_E_DEJA; return; }
  if (mode == 'I') {
    if (!SD.exists(nom)) { basErr = BAS_E_INTROUV; return; }
    basFile[i] = SD.open(nom, FILE_READ);
  }
  else if (mode == 'O') {
    if (SD.exists(nom)) SD.remove(nom);
    basFile[i] = SD.open(nom, FILE_WRITE);
  }
  else {
#ifdef FILE_APPEND
    basFile[i] = SD.open(nom, FILE_APPEND);  // ESP32 : FILE_WRITE efface le fichier
#else
    basFile[i] = SD.open(nom, FILE_WRITE);   // ATmega : FILE_WRITE écrit à la fin
#endif
  }
  if (!basFile[i]) basErr = BAS_E_FICHIER;
}

//////////////////////////////////////////////////////////////////////////////////////

void basClose()
{
  // CLOSE (tous les fichiers) ou CLOSE #n[,#m]
  basSkip();
  if (*basP == 0 || *basP == ':' || basAtElse()) { basCloseAll(); return; }
  while (basErr == 0) {
    byte i = basChannel(false);
    if (basErr) return;
    if (basFile[i]) basFile[i].close();
    basSkip();
    if (*basP != ',') break;
    basP++;
  }
}

//////////////////////////////////////////////////////////////////////////////////////

void basU8Trim(char* s, byte n)
{
  // Termine la chaîne s de n octets sans laisser de caractère UTF-8 coupé
  byte k = n;
  while (k > 0 && (((byte) s[k - 1]) & 0xC0) == 0x80) k--;  // Octets de suite
  if (k > 0 && (((byte) s[k - 1]) & 0x80) != 0) {          // Premier octet d'un caractère
    byte c = (byte) s[k - 1];
    byte attendu = ((c & 0xE0) == 0xC0) ? 2 : ((c & 0xF0) == 0xE0) ? 3 : 4;
    if (n - (k - 1) < attendu) n = k - 1;                  // Caractère incomplet : on l'enlève
  }
  s[n] = 0;
}

//////////////////////////////////////////////////////////////////////////////////////

boolean basFileField(File& f, char* out, boolean ligne)
{
  // Lit dans le fichier une valeur (jusqu'à une virgule ou une fin de ligne), ou une ligne
  // entière si ligne est vrai. Une valeur peut être entre guillemets (elle peut alors
  // contenir des virgules). Renvoie false si le fichier est déjà terminé.
  int c = f.read();
  if (!ligne) while (c == ' ') c = f.read();
  if (c < 0) return false;
  byte n = 0;
  boolean guillemets = (!ligne && c == '"');
  boolean ferme = false;  // Le guillemet de fin a été lu
  if (guillemets) c = f.read();
  while (c >= 0 && c != '\n') {
    if (c == '\r') { }
    else if (guillemets && !ferme) {
      if (c == '"') ferme = true;
      else if (n < BAS_STR_MAX) out[n++] = (char) c;
    }
    else if (!ligne && c == ',') break;
    else if (!ferme && n < BAS_STR_MAX) out[n++] = (char) c;  // Après le guillemet de fin : ignoré
    c = f.read();
  }
  if (!guillemets && !ligne) while (n > 0 && out[n - 1] == ' ') n--;  // Espaces de fin
  basU8Trim(out, n);
  return true;
}

//////////////////////////////////////////////////////////////////////////////////////

void basFileInput(boolean ligne)
{
  // INPUT #n, variable[,variable...]   ou   LINE INPUT #n, A$ (ligne vrai)
  basSkip();
  if (*basP != '#') { basErr = BAS_E_SYNTAXE; return; }
  byte i = basChannel(true);
  basExpect(',');
  if (basErr) return;
  while (basErr == 0) {
    basSkip();
    if (!basIsLetter(*basP) || basIsLetter(basP[1])) { basErr = BAS_E_SYNTAXE; return; }
    byte idx = *basP - 'A';
    basP++;
    boolean chaine = false;
    float* cible = &basVar[idx];
    if (*basP == '$') { basP++; chaine = true; }
    else if (*basP == '(') { cible = basArrRef(idx); if (cible == NULL) return; }
    if (ligne && !chaine) { basErr = BAS_E_TYPE; return; }
    if (!basFileField(basFile[i], basInBuf, ligne)) { basErr = BAS_E_FINFICH; return; }
    if (chaine) strcpy(basStr[idx], basInBuf);
    else *cible = basStrToNum(basInBuf);
    if (ligne) break;
    basSkip();
    if (*basP == ',') basP++;
    else break;
  }
}

//////////////////////////////////////////////////////////////////////////////////////
// Instructions
//////////////////////////////////////////////////////////////////////////////////////

boolean basListPages()
{
  // L'option /P de LIST (avant ou après les numéros de lignes)
  basSkip();
  if (*basP != '/') return false;
  basP++;
  basSkip();
  if (*basP != 'P') { basErr = BAS_E_SYNTAXE; return false; }
  basP++;
  return true;
}

//////////////////////////////////////////////////////////////////////////////////////

boolean basListPause()
{
  // LIST /P : l'écran est plein. On attend SUITE (ou ENVOI) pour continuer ;
  // ANNULATION ou SOMMAIRE arrête la liste (renvoie false).
  basKeyName(PSTR("SUITE"));
  basPuts(" : suite - ");
  basKeyName(PSTR("ANNULATION"));
  basPutsP(PSTR(" : arrêt"));
  unsigned long k = 0;
  while (k != SUITE && k != ENVOI && k != 0x0D && k != ANNULATION && k != SOMMAIRE) k = minitel.getKeyCode();
  minitel.writeByte(0x0D);       // On efface la ligne des touches
  minitel.clearLineFromCursor();
  basCol = 0;
  return k != ANNULATION && k != SOMMAIRE;
}

//////////////////////////////////////////////////////////////////////////////////////

void basList()
{
  // LIST [n][-m] [/P] : avec /P, la liste s'arrête à chaque écran plein
  unsigned int debut = 1;
  unsigned int fin = 65535;
  float v;
  boolean pages = basListPages();
  if (basErr) return;
  basSkip();
  if (basParseNum(&basP, &v)) { debut = (unsigned int) v; fin = debut; }
  basSkip();
  if (*basP == '-') {
    basP++;
    fin = 65535;
    basSkip();
    if (basParseNum(&basP, &v)) fin = (unsigned int) v;
  }
  if (!pages) pages = basListPages();
  if (basErr) return;
  char* suite = basP;  // basP ne doit pas changer pendant la liste si elle est lancée par un programme
  byte rangees = 0;    // Rangées de l'écran déjà remplies par la liste
  for (byte* p = basFindLine(debut); p < basProgEnd() && basLineNum(p) <= fin && basErr == 0; p += 3 + p[2]) {
    unsigned int n = basLineNum(p);
    byte chiffres = (n >= 10000) ? 5 : (n >= 1000) ? 4 : (n >= 100) ? 3 : (n >= 10) ? 2 : 1;
    // Rangées de cette ligne (une ligne de 40 caractères pile en prend deux : le Minitel passe
    // seul à la rangée suivante, puis vient le retour à la ligne)
    byte hauteur = (chiffres + 1 + basU8Len((char*) (p + 3))) / 40 + 1;
    if (pages && rangees + hauteur > 22) {
      if (!basListPause()) break;
      rangees = 0;
    }
    basPrintNum(n);
    basPuts(" ");
    basPuts((char*) (p + 3));
    basNewLine();
    rangees += hauteur;
    basCheckBreak();
  }
  basP = suite;
}

//////////////////////////////////////////////////////////////////////////////////////

void basPrintOut(File* f, const char* s, byte* col)
{
  // Ecrit pour PRINT : à l'écran (f NULL) ou dans un fichier
  if (f == NULL) { basPuts(s); return; }
  f->print(s);
  *col += basU8Len(s);
}

//////////////////////////////////////////////////////////////////////////////////////

void basPrint()
{
  // PRINT ... à l'écran, ou PRINT #n, ... dans un fichier
  File* f = NULL;
  byte colFichier = 0;
  basSkip();
  if (*basP == '#') {
    byte i = basChannel(true);
    if (basErr) return;
    f = &basFile[i];
    basSkip();
    if (*basP == ',') basP++;
    else if (*basP != 0 && *basP != ':' && !basAtElse()) { basErr = BAS_E_SYNTAXE; return; }
  }
  byte* col = (f == NULL) ? &basCol : &colFichier;
  boolean retourLigne = true;
  while (basErr == 0) {
    basSkip();
    char c = *basP;
    if (c == 0 || c == ':' || basAtElse()) break;
    if (c == ';') {
      basP++;
      retourLigne = false;
    }
    else if (c == ',') {
      basP++;
      retourLigne = false;
      if (f != NULL) {  // Dans un fichier : une virgule sépare les valeurs
        basPrintOut(f, ",", col);
      }
      else {            // A l'écran : colonne suivante multiple de 10
        do {
          minitel.writeByte(' ');
          basAvance();
        } while (basCol % 10 != 0);
      }
    }
    else if (BAS_KW("TAB")) {
      long n = (long) basArg();
      retourLigne = true;
      while (basErr == 0 && *col < n - 1 && (f != NULL || basCol < 39)) basPrintOut(f, " ", col);
    }
    else if (basIsStrStart()) {
      char s[BAS_STR_MAX + 1];
      basStrExpr(s);
      if (basErr == 0) basPrintOut(f, s, col);
      retourLigne = true;
    }
    else {
      float v = basExpr();
      if (basErr == 0) {
        char buf[20];
        basNumToStr(v, buf);
        basPrintOut(f, buf, col);
      }
      retourLigne = true;
    }
  }
  if (basErr == 0 && retourLigne) {
    if (f == NULL) basNewLine();
    else f->print("\r\n");
  }
}

//////////////////////////////////////////////////////////////////////////////////////

void basInput()
{
  basSkip();
  if (*basP == '#') { basFileInput(false); return; }
  if (*basP == '"') {  // Texte affiché avant la question
    char s[BAS_STR_MAX + 1];
    basStrPrimary(s);
    basPuts(s);
    basSkip();
    if (*basP == ';' || *basP == ',') basP++;
    else { basErr = BAS_E_SYNTAXE; return; }
  }
  while (basErr == 0) {
    // La variable à remplir
    basSkip();
    if (!basIsLetter(*basP) || basIsLetter(basP[1])) { basErr = BAS_E_SYNTAXE; return; }
    byte idx = *basP - 'A';
    basP++;
    boolean chaine = false;
    float* cible = &basVar[idx];
    if (*basP == '$') { basP++; chaine = true; }
    else if (*basP == '(') { cible = basArrRef(idx); if (cible == NULL) return; }
    // La saisie
    boolean correct = false;
    while (!correct) {
      basPuts("? ");
      basKey = 0;
      byte r = basReadLine(basInBuf, BAS_STR_MAX, false);
      basNewLine();
      if (r == BAS_K_QUITTER || r == BAS_K_ANNULER) { basErr = BAS_E_BREAK; return; }
      if (chaine) {
        strcpy(basStr[idx], basInBuf);
        correct = true;
      }
      else {
        char* q = basInBuf;
        while (*q == ' ') q++;
        boolean negatif = (*q == '-');
        if (*q == '-' || *q == '+') q++;
        float v;
        basUpper(q);  // Pour le E de l'exposant
        if (basParseNum(&q, &v)) {
          *cible = negatif ? -v : v;
          correct = true;
        }
        else {
          basPutsP(PSTR("Nombre attendu\n"));
        }
      }
    }
    basSkip();
    if (*basP == ',') basP++;
    else break;
  }
}

//////////////////////////////////////////////////////////////////////////////////////

void basFor()
{
  basSkip();
  if (!basIsLetter(*basP) || basIsLetter(basP[1])) { basErr = BAS_E_SYNTAXE; return; }
  byte idx = *basP - 'A';
  basP++;
  basExpect('=');
  float debut = basExpr();
  if (basErr) return;
  if (!BAS_KW("TO")) { basErr = BAS_E_SYNTAXE; return; }
  float limite = basExpr();
  float pas = 1;
  if (basErr == 0 && BAS_KW("STEP")) pas = basExpr();
  if (basErr) return;
  // Si une boucle sur la même variable est déjà en cours, elle est abandonnée
  for (byte i = 0; i < basForTop; i++) {
    if (basForVar[i] == idx) { basForTop = i; break; }
  }
  if (basForTop >= BAS_FOR_MAX) { basErr = BAS_E_IMBRIC; return; }
  basVar[idx] = debut;
  basForVar[basForTop] = idx;
  basForLimit[basForTop] = limite;
  basForStep[basForTop] = pas;
  basForLine[basForTop] = basCur;
  basForPos[basForTop] = basP;
  basForTop++;
}

//////////////////////////////////////////////////////////////////////////////////////

void basNext()
{
  basSkip();
  if (basIsLetter(*basP) && !basIsLetter(basP[1])) {  // NEXT avec le nom de la variable
    byte idx = *basP - 'A';
    basP++;
    while (basForTop > 0 && basForVar[basForTop - 1] != idx) basForTop--;
  }
  if (basForTop == 0) { basErr = BAS_E_NEXT; return; }
  byte i = basForTop - 1;
  float v = basVar[basForVar[i]] + basForStep[i];
  basVar[basForVar[i]] = v;
  if ((basForStep[i] >= 0 && v <= basForLimit[i]) || (basForStep[i] < 0 && v >= basForLimit[i])) {
    basCur = basForLine[i];  // On repart juste après le FOR
    basP = basForPos[i];
  }
  else {
    basForTop--;  // La boucle est terminée
  }
}

//////////////////////////////////////////////////////////////////////////////////////

void basLet()
{
  // Affectation : A = ...   A$ = ...   A(i) = ...
  basSkip();
  if (!basIsLetter(*basP) || basIsLetter(basP[1])) { basErr = BAS_E_SYNTAXE; return; }
  byte idx = *basP - 'A';
  basP++;
  if (*basP == '$') {
    basP++;
    basExpect('=');
    if (basErr) return;
    char s[BAS_STR_MAX + 1];
    basStrExpr(s);
    if (basErr == 0) strcpy(basStr[idx], s);
  }
  else if (*basP == '(') {
    float* element = basArrRef(idx);
    basExpect('=');
    if (basErr) return;
    float v = basExpr();
    if (basErr == 0) *element = v;
  }
  else {
    basExpect('=');
    if (basErr) return;
    float v = basExpr();
    if (basErr == 0) basVar[idx] = v;
  }
}

//////////////////////////////////////////////////////////////////////////////////////

void basGosub(unsigned int num)
{
  // Appel d'un sous-programme : on garde la position de retour
  if (basGsTop >= BAS_GOSUB_MAX) { basErr = BAS_E_IMBRIC; return; }
  basGsLine[basGsTop] = basCur;
  basGsPos[basGsTop] = basP;
  basGsTop++;
  basGoto(num);
  if (basErr) basGsTop--;
}

//////////////////////////////////////////////////////////////////////////////////////

void basIf()
{
  // IF condition THEN instruction(s) ou numéro [ELSE instruction(s) ou numéro]
  float v = basExpr();
  if (basErr) return;
  if (!BAS_KW("THEN")) {  // IF ... GOTO est accepté
    basSkip();
    if (!basWordIs(basP, PSTR("GOTO"))) { basErr = BAS_E_SYNTAXE; return; }
  }
  if (v == 0) {  // Condition fausse : on va au ELSE, ou à la ligne suivante s'il n'y en a pas
    char* suite = basFindElse(basP);
    if (suite == NULL) { basP += strlen(basP); return; }
    basP = suite;
  }
  basSkip();
  if (basIsDigit(*basP)) {
    unsigned int num = basLineArg();
    if (basErr == 0) basGoto(num);
  }
  else {
    basStatement();  // Les instructions suivantes de la branche sont exécutées par basRun
  }
}

//////////////////////////////////////////////////////////////////////////////////////

void basOn()
{
  // ON x GOTO n1,n2...   ON x GOSUB n1,n2...   (si x est hors de la liste : instruction suivante)
  long x = (long) basExpr();
  if (basErr) return;
  boolean sousProgramme;
  if (BAS_KW("GOTO")) sousProgramme = false;
  else if (BAS_KW("GOSUB")) sousProgramme = true;
  else { basErr = BAS_E_SYNTAXE; return; }
  unsigned int cible = 0;
  long rang = 1;
  while (true) {
    unsigned int num = basLineArg();
    if (basErr) return;
    if (rang == x) cible = num;
    basSkip();
    if (*basP != ',') break;
    basP++;
    rang++;
  }
  if (cible == 0) return;
  if (sousProgramme) basGosub(cible);  // Le retour se fera après toute la liste
  else basGoto(cible);
}

//////////////////////////////////////////////////////////////////////////////////////

boolean basFindWend(byte** ligne, char** pos)
{
  // Cherche le WEND qui correspond au WHILE, à partir de *pos dans *ligne (NULL : ligne
  // tapée au clavier). Renvoie false s'il n'y en a pas ; sinon *ligne et *pos donnent
  // la position qui suit ce WEND.
  byte niveau = 0;
  byte* l = *ligne;
  char* p = *pos;
  while (true) {
    p = basNextWord(p);
    while (p == NULL) {  // Fin de la ligne : on passe à la suivante
      if (l == NULL) return false;
      l += 3 + l[2];
      if (l >= basProgEnd()) return false;
      p = basNextWord((char*) (l + 3));
    }
    if (basWordIs(p, PSTR("WHILE"))) niveau++;
    else if (basWordIs(p, PSTR("WEND"))) {
      if (niveau == 0) { *ligne = l; *pos = p + 4; return true; }
      niveau--;
    }
    p = basWordEnd(p);
  }
}

//////////////////////////////////////////////////////////////////////////////////////

void basWhile(char* debut)
{
  // WHILE condition : debut est la position du mot WHILE, où WEND fait revenir
  float v = basExpr();
  if (basErr) return;
  if (v != 0) {
    for (byte i = 0; i < basWhTop; i++) {  // Cette boucle est déjà en cours (sortie par GOTO...)
      if (basWhPos[i] == debut) { basWhTop = i; break; }
    }
    if (basWhTop >= BAS_WHILE_MAX) { basErr = BAS_E_IMBRIC; return; }
    basWhLine[basWhTop] = basCur;
    basWhPos[basWhTop] = debut;
    basWhTop++;
  }
  else {  // Condition fausse : on continue après le WEND correspondant
    byte* ligne = basCur;
    char* pos = basP;
    if (!basFindWend(&ligne, &pos)) { basErr = BAS_E_WHILE; return; }
    basCur = ligne;
    basP = pos;
    basJumped = true;
  }
}

//////////////////////////////////////////////////////////////////////////////////////

char* basFindData(char* p)
{
  // Cherche une instruction DATA dans la suite de la ligne. Renvoie la position qui suit
  // le mot DATA, ou NULL.
  while (*p) {
    while (*p == ' ') p++;
    if (basWordIs(p, PSTR("DATA"))) return p + 4;
    if (basWordIs(p, PSTR("REM")) || *p == '\'') return NULL;
    p = basSkipStatement(p);
    if (*p == ':') p++;
  }
  return NULL;
}

//////////////////////////////////////////////////////////////////////////////////////

boolean basDataNext(char* out)
{
  // Lit la prochaine donnée des lignes DATA (out : BAS_STR_MAX + 1 octets)
  if (basDataLine == NULL) {
    basDataLine = basProg;
    basDataPos = (char*) (basProg + 3);
    basDataIn = false;
  }
  while (!basDataIn) {
    if (basDataLine >= basProgEnd()) { basErr = BAS_E_DATA; return false; }
    char* q = basFindData(basDataPos);
    if (q != NULL) {
      basDataPos = q;
      basDataIn = true;
    }
    else {
      basDataLine += 3 + basDataLine[2];
      basDataPos = (char*) (basDataLine + 3);
    }
  }
  char* p = basDataPos;
  while (*p == ' ') p++;
  out[0] = 0;
  if (*p == '"') {  // Donnée entre guillemets
    p++;
    char* debut = p;
    while (*p && *p != '"') p++;
    basStrAppend(out, debut, p);
    if (*p == '"') p++;
    while (*p == ' ') p++;
  }
  else {            // Donnée sans guillemets : jusqu'à la virgule, sans les espaces de fin
    char* debut = p;
    while (*p && *p != ',' && *p != ':') p++;
    char* fin = p;
    while (fin > debut && fin[-1] == ' ') fin--;
    basStrAppend(out, debut, fin);
  }
  if (*p == ',') p++;
  else {            // Fin de la liste de ce DATA
    basDataIn = false;
    if (*p == ':') p++;
  }
  basDataPos = p;
  return true;
}

//////////////////////////////////////////////////////////////////////////////////////

boolean basIsNumber(const char* s)
{
  // La chaîne contient-elle un nombre et rien d'autre (espaces exceptés) ?
  char* q = (char*) s;
  while (*q == ' ') q++;
  if (*q == '-' || *q == '+') q++;
  float v;
  if (!basParseNum(&q, &v)) return false;
  while (*q == ' ') q++;
  return (*q == 0);
}

//////////////////////////////////////////////////////////////////////////////////////

void basRead()
{
  // READ variable[,variable...]
  while (basErr == 0) {
    basSkip();
    if (!basIsLetter(*basP) || basIsLetter(basP[1])) { basErr = BAS_E_SYNTAXE; return; }
    byte idx = *basP - 'A';
    basP++;
    boolean chaine = false;
    float* cible = &basVar[idx];
    if (*basP == '$') { basP++; chaine = true; }
    else if (*basP == '(') { cible = basArrRef(idx); if (cible == NULL) return; }
    char donnee[BAS_STR_MAX + 1];
    if (!basDataNext(donnee)) return;
    if (chaine) strcpy(basStr[idx], donnee);
    else if (basIsNumber(donnee)) *cible = basStrToNum(donnee);
    else { basErr = BAS_E_TYPE; return; }
    basSkip();
    if (*basP == ',') basP++;
    else break;
  }
}

//////////////////////////////////////////////////////////////////////////////////////

void basRestore()
{
  // RESTORE (premier DATA du programme) ou RESTORE n (premier DATA à partir de la ligne n)
  basSkip();
  basDataIn = false;
  if (*basP == 0 || *basP == ':' || basAtElse()) {
    basDataLine = NULL;
    return;
  }
  unsigned int num = basLineArg();
  if (basErr) return;
  byte* p = basFindLine(num);
  if (p >= basProgEnd() || basLineNum(p) != num) { basErr = BAS_E_LIGNE; basErrLigne = num; return; }
  basDataLine = p;
  basDataPos = (char*) (p + 3);
}

//////////////////////////////////////////////////////////////////////////////////////

void basPinWrite(char quoi)
{
  // PINMODE (quoi 'M'), DWRITE ('D') ou AWRITE ('A') : broche,valeur
  long broche = (long) basExpr();
  basExpect(',');
  long v = (long) basExpr();
  if (basErr) return;
  if (!basPinOK(broche)) { basErr = BAS_E_BROCHE; return; }
  if (quoi == 'M') {
    if (v == 0) pinMode((int) broche, INPUT);
    else if (v == 1) pinMode((int) broche, OUTPUT);
    else if (v == 2) pinMode((int) broche, INPUT_PULLUP);
    else basErr = BAS_E_VALEUR;
  }
  else if (quoi == 'D') {
    digitalWrite((int) broche, (v != 0) ? HIGH : LOW);
  }
  else {
    if (v < 0 || v > 255) { basErr = BAS_E_VALEUR; return; }
    analogWrite((int) broche, (int) v);
  }
}

//////////////////////////////////////////////////////////////////////////////////////

long basRenumMap(unsigned long ancien, unsigned int debut, unsigned int pas)
{
  // Nouveau numéro de la ligne ancien (-1 si elle n'existe pas)
  long rang = 0;
  for (byte* p = basProg; p < basProgEnd(); p += 3 + p[2], rang++) {
    unsigned int num = basLineNum(p);
    if (num == ancien) return debut + rang * pas;
    if (num > ancien) break;
  }
  return -1;
}

//////////////////////////////////////////////////////////////////////////////////////

int basRenumText(const char* src, char* dst, unsigned int debut, unsigned int pas)
{
  // Recopie dans dst le texte d'une ligne en remplaçant les numéros de ligne qui suivent
  // GOTO, GOSUB, THEN, ELSE et RESTORE. Renvoie la longueur obtenue, ou -1 si elle
  // dépasse BAS_TEXT_MAX. Un numéro de ligne inexistante est laissé tel quel.
  int n = 0;
  const char* p = src;
  while (*p) {
    if (n >= BAS_TEXT_MAX) return -1;
    if (*p == '"') {  // Chaîne : recopiée telle quelle
      dst[n++] = *p++;
      while (*p && *p != '"') { if (n >= BAS_TEXT_MAX) return -1; dst[n++] = *p++; }
      if (*p == '"') { if (n >= BAS_TEXT_MAX) return -1; dst[n++] = *p++; }
      continue;
    }
    if (*p == '\'' || basWordIs(p, PSTR("REM"))) {  // Commentaire : la fin de la ligne est recopiée
      while (*p) { if (n >= BAS_TEXT_MAX) return -1; dst[n++] = *p++; }
      break;
    }
    if (!basIsLetter(*p)) { dst[n++] = *p++; continue; }
    // Un mot : est-ce un mot-clé suivi d'un numéro de ligne ?
    byte lg = 0;
    boolean liste = false;  // Après GOTO et GOSUB, une liste (ON x GOTO n1,n2...)
    if (basWordIs(p, PSTR("GOTO"))) { lg = 4; liste = true; }
    else if (basWordIs(p, PSTR("GOSUB"))) { lg = 5; liste = true; }
    else if (basWordIs(p, PSTR("THEN")) || basWordIs(p, PSTR("ELSE"))) lg = 4;
    else if (basWordIs(p, PSTR("RESTORE"))) lg = 7;
    if (lg == 0) {  // Autre mot : recopié
      while (basIsLetter(*p)) { if (n >= BAS_TEXT_MAX) return -1; dst[n++] = *p++; }
      continue;
    }
    while (lg-- > 0) { if (n >= BAS_TEXT_MAX) return -1; dst[n++] = *p++; }
    while (true) {
      while (*p == ' ') { if (n >= BAS_TEXT_MAX) return -1; dst[n++] = *p++; }
      if (!basIsDigit(*p)) break;
      const char* debutNum = p;
      unsigned long ancien = 0;
      while (basIsDigit(*p)) { if (ancien < 100000) ancien = ancien * 10 + (*p - '0'); p++; }
      long nouveau = basRenumMap(ancien, debut, pas);
      if (nouveau >= 0) {
        char tmp[8];
        basUtoa((unsigned long) nouveau, tmp);
        for (char* t = tmp; *t; t++) { if (n >= BAS_TEXT_MAX) return -1; dst[n++] = *t; }
      }
      else {
        for (const char* t = debutNum; t < p; t++) { if (n >= BAS_TEXT_MAX) return -1; dst[n++] = *t; }
      }
      if (!liste) break;
      const char* q = p;
      while (*q == ' ') q++;
      if (*q != ',') break;
      while (p <= q) { if (n >= BAS_TEXT_MAX) return -1; dst[n++] = *p++; }  // Espaces et virgule
    }
  }
  dst[n] = 0;
  return n;
}

//////////////////////////////////////////////////////////////////////////////////////

void basRenum()
{
  // RENUM [début][,pas]
  long debut = 10;
  long pas = 10;
  basSkip();
  if (*basP != 0 && *basP != ':' && *basP != ',') debut = (long) basExpr();
  basSkip();
  if (basErr == 0 && *basP == ',') {
    basP++;
    pas = (long) basExpr();
  }
  if (basErr) return;
  if (debut < 1 || debut > 65535 || pas < 1 || pas > 65535) { basErr = BAS_E_VALEUR; return; }
  long lignes = 0;
  for (byte* p = basProg; p < basProgEnd(); p += 3 + p[2]) lignes++;
  if (lignes == 0) return;
  if (debut + (lignes - 1) * pas > 65535) { basErr = BAS_E_VALEUR; return; }
  char tmp[BAS_TEXT_MAX + 1];
  // 1. On vérifie d'abord que tout tiendra en mémoire (sinon rien n'est changé)
  // (les lignes sont réécrites une à une : la taille ne doit dépasser la place à aucune étape)
  long total = basProgLen;
  long maxi = total;
  for (byte* p = basProg; p < basProgEnd(); p += 3 + p[2]) {
    int len = basRenumText((char*) (p + 3), tmp, debut, pas);
    if (len < 0) { basErr = BAS_E_MEMOIRE; return; }
    total += len - (p[2] - 1);
    if (total > maxi) maxi = total;
  }
  if (maxi > BAS_PROG_SIZE) { basErr = BAS_E_MEMOIRE; return; }
  // 2. On réécrit les textes (les anciens numéros servent encore de références)
  for (byte* p = basProg; p < basProgEnd(); ) {
    int len = basRenumText((char*) (p + 3), tmp, debut, pas);
    unsigned int ancienne = p[2];
    unsigned int nouvelle = len + 1;
    if (nouvelle != ancienne) {
      memmove(p + 3 + nouvelle, p + 3 + ancienne, basProgEnd() - (p + 3 + ancienne));
      basProgLen = basProgLen + nouvelle - ancienne;
    }
    memcpy(p + 3, tmp, nouvelle);
    p[2] = nouvelle;
    p += 3 + nouvelle;
  }
  // 3. On change les numéros des lignes
  long num = debut;
  for (byte* p = basProg; p < basProgEnd(); p += 3 + p[2], num += pas) {
    p[0] = num & 0xFF;
    p[1] = num >> 8;
  }
  basResetStacks();
  basStop = true;  // Le programme a changé : on n'exécute rien de plus
}

//////////////////////////////////////////////////////////////////////////////////////

void basStatement()
{
  basSkip();
  char* debut = basP;  // Début de l'instruction (pour WHILE)
  if (strncmp_P(basP, PSTR("REM"), 3) == 0 || *basP == '\'') {  // Commentaire : on saute la fin de la ligne
    basP += strlen(basP);
  }
  else if (BAS_KW("ELSE")) {  // Fin de la branche THEN exécutée : on saute la branche ELSE
    basP += strlen(basP);
  }
  else if (*basP == '?') { basP++; basPrint(); }
  else if (BAS_KW("PRINT")) basPrint();
  else if (BAS_KW("INPUT")) basInput();
  else if (BAS_KW("IF")) basIf();
  else if (BAS_KW("GOTO")) {
    unsigned int num = basLineArg();
    if (basErr == 0) basGoto(num);
  }
  else if (BAS_KW("GOSUB")) {
    unsigned int num = basLineArg();
    if (basErr == 0) basGosub(num);
  }
  else if (BAS_KW("ON")) basOn();
  else if (BAS_KW("RETURN")) {
    if (basGsTop == 0) { basErr = BAS_E_RETURN; return; }
    basGsTop--;
    basCur = basGsLine[basGsTop];
    basP = basGsPos[basGsTop];
  }
  else if (BAS_KW("FOR")) basFor();
  else if (BAS_KW("NEXT")) basNext();
  else if (BAS_KW("WHILE")) basWhile(debut);
  else if (BAS_KW("WEND")) {
    if (basWhTop == 0) { basErr = BAS_E_WEND; return; }
    basWhTop--;
    basCur = basWhLine[basWhTop];  // On revient au WHILE, qui teste à nouveau la condition
    basP = basWhPos[basWhTop];
    basJumped = true;
  }
  else if (BAS_KW("DATA")) basP = basSkipStatement(basP);
  else if (BAS_KW("READ")) basRead();
  else if (BAS_KW("RESTORE")) basRestore();
  else if (BAS_KW("END")) {
    basCloseAll();
    basStop = true;
  }
  else if (BAS_KW("STOP")) basStop = true;
  else if (BAS_KW("DIM")) {
    boolean encore = true;
    while (encore && basErr == 0) {
      basSkip();
      if (!basIsLetter(*basP) || basP[1] != '(') { basErr = BAS_E_SYNTAXE; return; }
      byte idx = *basP - 'A';
      basP++;
      long n = (long) basArg();
      if (basErr == 0) basDim(idx, n);
      basSkip();
      encore = (*basP == ',');  // Plusieurs tableaux séparés par des virgules
      if (encore) basP++;
    }
  }
  else if (BAS_KW("CLS")) {
    minitel.newScreen();  // Le Minitel revient aussi à l'affichage normal
    basCol = 0;
    basGfxClear();
    basInk = 7;
    basPaper = 0;
    basStyle = 0;
  }
  else if (BAS_KW("PLOT") || BAS_KW("LINE") || BAS_KW("BOX") || BAS_KW("CIRCLE")) {
    char genre = debut[0];  // 'P' : PLOT, 'L' : LINE, 'B' : BOX, 'C' : CIRCLE
    if (genre == 'L' && BAS_KW("INPUT")) { basFileInput(true); return; }  // LINE INPUT #n, A$
    long c[4];
    byte n = (genre == 'P') ? 2 : (genre == 'C') ? 3 : 4;
    for (byte i = 0; i < n && basErr == 0; i++) {
      if (i > 0) basExpect(',');
      float v = basExpr();
      if (v < -1000 || v > 1000) { if (basErr == 0) basErr = BAS_E_VALEUR; }
      c[i] = (long) v;
    }
    boolean allume = true;   // Paramètre facultatif : 0 pour éteindre
    boolean plein = false;   // BOX et CIRCLE : 1 pour une figure pleine
    basSkip();
    if (basErr == 0 && *basP == ',') {
      basP++;
      allume = (basExpr() != 0);
      basSkip();
      if (basErr == 0 && *basP == ',' && (genre == 'B' || genre == 'C')) {
        basP++;
        plein = (basExpr() != 0);
      }
    }
    if (basErr) return;
    if (genre == 'C' && c[2] < 0) { basErr = BAS_E_VALEUR; return; }
    if (genre == 'P') {
      if (c[0] >= 0 && c[0] < BAS_GFX_W && c[1] >= 0 && c[1] < BAS_GFX_H) {
        basGfxSet((int) c[0], (int) c[1], allume);
        basGfxCell((int) c[0] / 2, (int) c[1] / 3);
      }
    }
    else if (genre == 'L') basGfxLine(c[0], c[1], c[2], c[3], allume);
    else if (genre == 'B') basGfxBox(c[0], c[1], c[2], c[3], allume, plein);
    else basGfxCircle(c[0], c[1], c[2], allume, plein);
  }
  else if (BAS_KW("LOCATE")) {
    long x = (long) basExpr();
    basExpect(',');
    long y = (long) basExpr();
    if (basErr) return;
    if (x < 1 || x > 40 || y < 1 || y > 24) { basErr = BAS_E_VALEUR; return; }
    minitel.moveCursorXY((int) x, (int) y);
    basCol = (byte) (x - 1);
  }
  else if (BAS_KW("COLOR") || BAS_KW("PAPER") || BAS_KW("SIZE")) {
    char genre = debut[0];  // 'C' : COLOR, 'P' : PAPER, 'S' : SIZE
    long n = (long) basExpr();
    if (basErr) return;
    if (n < 0 || n > ((genre == 'S') ? 3 : 7)) { basErr = BAS_E_VALEUR; return; }
    if (genre == 'C') {
      minitel.attributs(CARACTERE_NOIR + (byte) n);  // 0 noir, 1 rouge, 2 vert, 3 jaune, 4 bleu, 5 magenta, 6 cyan, 7 blanc
      basInk = (byte) n;
    }
    else if (genre == 'P') {
      minitel.attributs(FOND_NOIR + (byte) n);  // Pris en compte au prochain espace (attribut de zone)
      basPaper = (byte) n;
    }
    else {
      // 0 normale, 1 double hauteur, 2 double largeur, 3 double taille. Pour passer en
      // double hauteur, minitel.attributs() descend le curseur d'une rangée : le texte
      // occupe alors la rangée de départ et la suivante.
      boolean hautAvant = (basStyle & BAS_ST_HAUT);
      boolean haut = (n & 1);
      if (haut && !hautAvant) minitel.attributs(GRANDEUR_NORMALE + (byte) n);
      else basEsc(GRANDEUR_NORMALE + (byte) n);
      basStyle = (basStyle & 0x0F) | ((byte) n << 4);
    }
    basAttr = true;
  }
  else if (BAS_KW("INVERSE") || BAS_KW("FLASH") || BAS_KW("UNDERLINE")) {
    char genre = debut[0];  // 'I' : INVERSE, 'F' : FLASH, 'U' : UNDERLINE
    long n = (long) basExpr();
    if (basErr) return;
    byte bit = (genre == 'I') ? BAS_ST_INVERSE : (genre == 'F') ? BAS_ST_FLASH : BAS_ST_SOULIGNE;
    if (genre == 'I') minitel.attributs((n != 0) ? INVERSION_FOND : FOND_NORMAL);
    else if (genre == 'F') minitel.attributs((n != 0) ? CLIGNOTEMENT : FIXE);
    else minitel.attributs((n != 0) ? DEBUT_LIGNAGE : FIN_LIGNAGE);  // Au prochain espace (attribut de zone)
    if (n != 0) basStyle |= bit;
    else basStyle &= ~bit;
    basAttr = true;
  }
  else if (BAS_KW("EDIT")) {
    // Prépare la ligne n pour la prochaine saisie : elle sera affichée dans la zone de
    // saisie, où l'on peut la corriger avant de la valider avec ENVOI
    unsigned int num = basLineArg();
    if (basErr) return;
    byte* p = basFindLine(num);
    if (p >= basProgEnd() || basLineNum(p) != num) { basErr = BAS_E_LIGNE; basErrLigne = num; return; }
    char* fin = basUtoa(num, basInBuf);
    *fin++ = ' ';
    if ((fin - basInBuf) + strlen((char*) (p + 3)) > BAS_LINE_MAX) { basErr = BAS_E_LONGUE; return; }
    strcpy(fin, (char*) (p + 3));
    basEditPending = true;
    basStop = true;
  }
  else if (BAS_KW("BEEP")) minitel.bip();
  else if (BAS_KW("SOUND")) {
    long frequence = (long) basExpr();
    basExpect(',');
    long duree = (long) basExpr();
    if (basErr) return;
    if (frequence < 31 || frequence > 20000 || duree < 0 || duree > 60000) { basErr = BAS_E_VALEUR; return; }
    tone(BUZZER, (unsigned int) frequence, duree);
    basWait(duree);
    noTone(BUZZER);
  }
  else if (BAS_KW("PAUSE")) {
    long duree = (long) basExpr();
    if (basErr) return;
    if (duree < 0) { basErr = BAS_E_VALEUR; return; }
    basWait(duree);
  }
  else if (BAS_KW("RUN")) {
    basCloseAll();
    basClearVars();
    basTimer0 = millis();
    randomSeed(micros());
    if (basProgLen == 0) { basStop = true; return; }
    basCur = basProg;
    basP = (char*) (basProg + 3);
    basJumped = true;
  }
  else if (BAS_KW("LIST")) basList();
  else if (BAS_KW("NEW")) {
    basCloseAll();
    basProgLen = 0;
    basClearVars();
    basStop = true;
  }
  else if (BAS_KW("MEM")) {
    basPrintNum(BAS_PROG_SIZE - basProgLen);
    basPutsP(PSTR(" octets libres\n"));
  }
  else if (BAS_KW("AIDE") || BAS_KW("HELP")) basHelp();
  else if (BAS_KW("SAVE")) basSave();
  else if (BAS_KW("LOAD")) basLoad(false);
  else if (BAS_KW("MERGE")) basLoad(true);
  else if (BAS_KW("RENUM")) basRenum();
  else if (BAS_KW("OPEN")) basOpen();
  else if (BAS_KW("CLOSE")) basClose();
  else if (BAS_KW("PINMODE")) basPinWrite('M');
  else if (BAS_KW("DWRITE")) basPinWrite('D');
  else if (BAS_KW("AWRITE")) basPinWrite('A');
  else if (BAS_KW("FILES")) basFiles();
  else if (BAS_KW("KILL")) {
    char nom[14];
    if (!basFileName(nom, PSTR(".BAS"), NULL)) return;
    if (!SD.exists(nom)) basErr = BAS_E_INTROUV;
    else if (!SD.remove(nom)) basErr = BAS_E_FICHIER;
  }
  else {
    BAS_KW("LET");  // LET est facultatif
    basLet();
  }
}

//////////////////////////////////////////////////////////////////////////////////////

void basRun()
{
  // Exécute les instructions à partir de basP, jusqu'à la fin de la ligne tapée
  // au clavier ou jusqu'à la fin du programme.
  while (basErr == 0 && !basStop) {
    basCheckBreak();
    if (basErr) break;
    basSkip();
    if (*basP == ':') {
      basP++;
    }
    else if (*basP == 0) {  // Fin de la ligne : on passe à la suivante
      if (basCur == NULL) break;
      basCur += 3 + basCur[2];
      if (basCur >= basProgEnd()) {  // Fin du programme
        basCloseAll();
        break;
      }
      basP = (char*) (basCur + 3);
    }
    else {
      basJumped = false;
      basDepth = 0;
      basStatement();
      if (basErr || basStop) break;
      if (!basJumped) {
        basSkip();
        if (*basP != 0 && *basP != ':' && !basAtElse()) basErr = BAS_E_SYNTAXE;  // Il reste quelque chose d'incompris
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////////////////////

void basExecuter(char* p)
{
  // Exécute tout de suite une ligne sans numéro (déjà en majuscules), comme si on l'avait
  // tapée, puis affiche « Ok »
  basP = p;
  basStop = false;
  basAttr = false;
  basKey = 0;
  basGsTop = 0;
  basForTop = 0;
  basWhTop = 0;
  basRun();
  if (basErr) basPrintError();
  if (basCur != NULL) basCloseAll();  // Un programme s'est arrêté (fin, erreur...) : ses fichiers sont fermés
  if (basAttr) basResetAttr();  // On revient à l'affichage normal
  if (basCol != 0) basNewLine();
  if (!basEditPending) basPutsP(PSTR("Ok\n"));  // Après EDIT, la ligne à corriger s'affiche aussitôt
}

//////////////////////////////////////////////////////////////////////////////////////

char basLancer[BAS_STR_MAX + 1] = "";  // Programme à charger et lancer en entrant (menu 3)

void basic()
{
  minitel.newScreen();
  minitel.echo(false);   // C'est le BASIC qui affiche ce qui est tapé au clavier
  minitel.scrollMode();  // L'écran défile quand on arrive en bas
  basCol = 0;
  basGfxClear();
  if (basProgLen == 0) basClearVars();  // Le programme est conservé quand on quitte le BASIC
  basTimer0 = millis();
  basPutsP(PSTR("reBASIC - "));
  basPrintNum(BAS_PROG_SIZE - basProgLen);
  basPutsP(PSTR(" octets libres\n"));
  basKeyName(PSTR("GUIDE"));
  basPutsP(PSTR(" : aide - "));
  basKeyName(PSTR("SOMMAIRE"));
  basPutsP(PSTR(" : retour au menu"));
  if (basCol != 0) basNewLine();  // 40 caractères pile : le Minitel est déjà passé à la ligne
  basPutsP(PSTR("Ok\n"));

  if (basLancer[0]) {
    // Le gestionnaire de la carte SD a demandé de lancer un programme : LOAD puis RUN,
    // affichés comme si on les avait tapés
    snprintf_P(basLine, BAS_LINE_MAX + 1, PSTR("LOAD \"%s\""), basLancer);
    basLancer[0] = 0;
    basPuts(basLine);
    basNewLine();
    basErr = 0;
    basCur = NULL;
    basExecuter(basLine);
    if (basErr == 0) {
      strcpy_P(basLine, PSTR("RUN"));
      basPuts(basLine);
      basNewLine();
      basCur = NULL;
      basExecuter(basLine);
    }
  }

  boolean fin = false;
  while (!fin) {
    boolean edition = basEditPending;  // EDIT n : la ligne à corriger est dans basInBuf
    basEditPending = false;
    if (edition) strcpy(basLine, basInBuf);
    byte r = basReadLine(basLine, BAS_LINE_MAX, edition);
    if (r == BAS_K_QUITTER) {
      fin = true;
    }
    else if (r == BAS_K_ANNULER) {
      basNewLine();
    }
    else if (r == BAS_K_AIDE) {
      basHelp();
      basPutsP(PSTR("Ok\n"));
    }
    else {
      basNewLine();
      basUpper(basLine);
      char* p = basLine;
      while (*p == ' ') p++;
      basErr = 0;
      basCur = NULL;
      if (basIsDigit(*p)) {  // Ligne de programme
        basEnterLine(p);
        if (basErr) basPrintError();
      }
      else if (*p != 0) {    // Exécution immédiate
        basExecuter(p);
      }
    }
  }

  basCloseAll();
  minitel.noCursor();
  minitel.pageMode();  // Retour au mode page pour les autres écrans
}

//////////////////////////////////////////////////////////////////////////////////////
