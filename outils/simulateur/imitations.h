//////////////////////////////////////////////////////////////////////////////////////
// Simulateur de reBASIC : imitations de l'Arduino, du Minitel, de la carte SD et de
// l'horloge, pour compiler Basic.ino sur un ordinateur.
//////////////////////////////////////////////////////////////////////////////////////
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <strings.h>
#include <poll.h>

typedef unsigned char byte;
typedef bool boolean;

// Mémoire programme (PROGMEM) : sur PC, tout est en mémoire vive
#define PSTR(s) (s)
#define PROGMEM
#define strncpy_P strncpy
#define strcpy_P strcpy
#define strlen_P strlen
#define strncmp_P strncmp
#define strcasecmp_P strcasecmp
#define strcmp_P strcmp
#define snprintf_P snprintf
#define pgm_read_byte(p) (*(const unsigned char*)(p))
#define pgm_read_word(p) (*(const unsigned short*)(p))

#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#define NUM_DIGITAL_PINS 70
#define FILE_READ 0
#define FILE_WRITE 1

// Constantes de Minitel1B_Hard
#define CARACTERE_NOIR 0x40
#define CARACTERE_BLANC 0x47
#define CLIGNOTEMENT 0x48
#define FIXE 0x49
#define GRANDEUR_NORMALE 0x4C
#define DOUBLE_HAUTEUR 0x4D
#define DOUBLE_LARGEUR 0x4E
#define DOUBLE_GRANDEUR 0x4F
#define FOND_NOIR 0x50
#define FIN_LIGNAGE 0x59
#define DEBUT_LIGNAGE 0x5A
#define FOND_NORMAL 0x5C
#define INVERSION_FOND 0x5D
#define ENVOI 0x1341
#define RETOUR 0x1342
#define GUIDE 0x1344
#define ANNULATION 0x1345
#define SOMMAIRE 0x1346
#define CORRECTION 0x1347
#define SUITE 0x1348

// Temps simulé : il avance d'une milliseconde à chaque lecture du clavier,
// c'est-à-dire à chaque instruction exécutée par reBASIC.
extern unsigned long g_ms;
inline unsigned long millis() { return g_ms; }
inline unsigned long micros() { return g_ms * 1000; }
inline long random(long a, long b) { return a + rand() % (b - a); }
inline void randomSeed(unsigned long) { }   // Tirages reproductibles d'une exécution à l'autre

// Buzzer et broches : les appels sont seulement notés dans un journal
extern std::string g_journal;
inline void journal(const char* f, long a, long b) { char t[64]; snprintf(t, sizeof t, f, a, b); g_journal += t; }
inline void tone(int, unsigned f, long d) { journal("[SOUND %ld %ld]", f, d); }
inline void noTone(int) { }
inline void pinMode(int p, int m) { journal("[PINMODE %ld %ld]", p, m); }
inline void digitalWrite(int p, int v) { journal("[DWRITE %ld %ld]", p, v); }
inline int digitalRead(int) { return (g_ms / 3000) % 2; }                     // Bouton simulé : change toutes les 3 s
inline int analogRead(int) { return (int) (512 + 511 * sin(g_ms / 2000.0)); } // Potentiomètre simulé
inline void analogWrite(int p, int v) { journal("[AWRITE %ld %ld]", p, v); }

struct String {
  std::string s;
  String(const char* c = "") : s(c) { }
  String(std::string x) : s(x) { }
  unsigned length() const { return s.size(); }
  const char* c_str() const { return s.c_str(); }
};

// Une case de l'écran du Minitel
struct Case {
  std::string car = " ";
  int couleur = 7, fond = 0;          // Couleurs du caractère et du fond (0 à 7)
  bool inverse = false, clignote = false, souligne = false;
  bool graphique = false; int points = 0;
  int taille = 0;                     // 0 normale, 1 double hauteur, 2 double largeur, 3 double taille
  bool couverte = false;              // Partie d'un grand caractère dessiné depuis une autre case
  int zoneFond = 0; bool zoneSouligne = false;   // Attributs de zone en vigueur dans cette case
  bool delim = false;                 // La case délimite une zone : elle a son propre fond
};

// Une touche du script de clavier
struct Touche { unsigned long code; bool toujours; int special; };
extern std::vector<Touche> g_touches;
extern size_t g_prochaine;
extern bool g_curseur;
extern int g_bips;
void capture(bool forcer);

// Le Minitel : un écran de 40 x 24 cases en mode rouleau. Les règles des attributs sont
// celles du STUM 1B (partie 2, chapitre 2) :
// - couleur, clignotement, inversion et taille s'appliquent aux caractères qui suivent ;
// - le fond et le soulignement sont des attributs de zone : ils restent « latents » jusqu'au
//   prochain espace (délimiteur) ; un caractère semi-graphique valide aussi le fond, et
//   délimite lui aussi une zone ;
// - une case qui n'est pas un délimiteur (lettre, espace sans changement, case jamais
//   écrite) montre le fond du dernier délimiteur à sa gauche, au moment de l'affichage :
//   récrire un délimiteur change le fond des cases qui le suivent, jusqu'au suivant
//   (constaté sur un Minitel 1B Alcatel) ;
// - le passage en semi-graphique annule la taille, l'inversion et le soulignement ;
// - la double hauteur est dessinée vers le haut, et ignorée sur la première rangée.
struct Minitel {
  Case ecran[24][40];
  int r = 0, c = 0;
  int couleur = 7, taille = 0;
  bool inverse = false, clignote = false, modeGraphique = false, esc = false;
  int fondLatent = 0, fondZone = 0;
  bool soulLatent = false, soulZone = false;
  bool delimiteurAttendu = false;   // Un attribut de zone vient d'être reçu : le prochain espace est un délimiteur
  void initAttributs() { couleur = 7; taille = 0; inverse = clignote = modeGraphique = esc = false; fondLatent = fondZone = 0; soulLatent = soulZone = false; delimiteurAttendu = false; }
  void effacer() { for (auto& l : ecran) for (auto& x : l) x = Case(); r = c = 0; initAttributs(); }
  void nouvelleRangee() { fondZone = 0; soulZone = false; }   // Une zone finit avec la rangée
  void zoneIci() {  // Après un déplacement : les attributs de zone sont ceux de la case de gauche
    if (c > 0) { fondZone = ecran[r][c - 1].zoneFond; soulZone = ecran[r][c - 1].zoneSouligne; }
    else nouvelleRangee();
  }
  void propager(int rr, int cc) {
    // Le fond d'une case qui n'est pas un délimiteur est celui du dernier délimiteur à sa
    // gauche : on le recalcule pour la case écrite et celles qui la suivent
    Case* l = ecran[rr];
    int z = l[cc].delim ? l[cc].fond : (cc > 0 ? l[cc - 1].zoneFond : 0);
    for (int j = cc; j < 40; j++) {
      if (j > cc && l[j].delim) break;
      l[j].fond = z; l[j].zoneFond = z;
    }
  }
  void defiler() {
    for (int i = 0; i < 23; i++) for (int j = 0; j < 40; j++) ecran[i][j] = ecran[i + 1][j];
    for (int j = 0; j < 40; j++) ecran[23][j] = Case();
    r = 23;
  }
  void descendre() { if (++r >= 24) defiler(); nouvelleRangee(); }
  void avancer(int n) { c += n; if (c >= 40) { c -= 40; descendre(); } }
  void poser(const std::string& car, bool graphique, int points) {
    bool delimiteur = false;
    if (graphique) {                                      // Un semi-graphique valide le fond
      fondZone = fondLatent;
      delimiteurAttendu = false;
      delimiteur = true;
    }
    else if (car == " " && (delimiteurAttendu || fondLatent != fondZone || soulLatent != soulZone)) {
      fondZone = fondLatent; soulZone = soulLatent;      // Délimiteur de zone
      delimiteurAttendu = false;
      delimiteur = true;
    }
    int t = graphique ? 0 : taille;
    int larg = (t & 2) ? 2 : 1, haut = (t & 1) ? 2 : 1;
    for (int dy = 0; dy < haut; dy++) {
      for (int dx = 0; dx < larg; dx++) {
        int rr = r - dy, cc = c + dx;
        if (rr < 0 || cc > 39) continue;
        Case& x = ecran[rr][cc];
        x = Case();
        x.couverte = (dx || dy);
        x.car = car; x.couleur = couleur; x.fond = fondZone;
        x.inverse = graphique ? false : inverse; x.clignote = clignote;
        x.souligne = (graphique || delimiteur) ? false : soulZone;   // Un délimiteur n'est pas souligné
        x.graphique = graphique; x.points = points; x.taille = t;
        x.zoneFond = fondZone; x.zoneSouligne = soulZone;
        x.delim = delimiteur && !dx && !dy;
        propager(rr, cc);
      }
    }
    avancer(larg);
  }
  void attribut(byte a) {
    if (a >= 0x40 && a <= 0x47) couleur = a - 0x40;
    else if (a == 0x48) clignote = true;
    else if (a == 0x49) clignote = false;
    else if (a >= 0x4C && a <= 0x4F) {
      if (modeGraphique) return;                              // Pas de taille en semi-graphique
      if ((a == 0x4D || a == 0x4F) && r + 1 <= 1) return;     // Ignorée sur la rangée 1
      taille = a - 0x4C;
    }
    else if (a >= 0x50 && a <= 0x57) { fondLatent = a - 0x50; delimiteurAttendu = true; }
    else if (a == 0x59) { soulLatent = false; delimiteurAttendu = true; }
    else if (a == 0x5A) { if (!modeGraphique) { soulLatent = true; delimiteurAttendu = true; } }
    else if (a == 0x5C) { if (!modeGraphique) inverse = false; }
    else if (a == 0x5D) { if (!modeGraphique) inverse = true; }
  }
  // Le décodeur du flux Vidéotex, comme la classe Ecran du Minitel virtuel (rebasic.js) :
  // reBASIC envoie tout octet par octet (writeByte), le reste du croquis passe aussi par
  // les fonctions de Minitel1B_Hard imitées plus bas.
  int etat = 0, memo = 0, saut = 0;
  std::string csi;
  std::string dernierCar = " "; bool dernierGraph = false; int dernierPoints = 0; bool aDernier = false;
  bool rangee0 = false; int avant0r = 0, avant0c = 0;
  static const char* g2simple(int b) {
    switch (b) {
      case 0x23: return "£"; case 0x27: return "§"; case 0x2C: return "←"; case 0x2D: return "↑";
      case 0x2E: return "→"; case 0x2F: return "↓"; case 0x30: return "°"; case 0x31: return "±";
      case 0x38: return "÷"; case 0x3C: return "¼"; case 0x3D: return "½"; case 0x3E: return "¾";
      case 0x6A: return "Œ"; case 0x7A: return "œ"; case 0x7B: return "ß";
    }
    return nullptr;
  }
  static std::string g2compose(int d, int l) {
    const char* r = nullptr;
    if (d == 0x41) r = (l == 'a') ? "à" : (l == 'e') ? "è" : (l == 'u') ? "ù" : nullptr;
    else if (d == 0x42) r = (l == 'e') ? "é" : nullptr;
    else if (d == 0x43) r = (l == 'a') ? "â" : (l == 'e') ? "ê" : (l == 'i') ? "î" : (l == 'o') ? "ô" : (l == 'u') ? "û" : nullptr;
    else if (d == 0x48) r = (l == 'a') ? "ä" : (l == 'e') ? "ë" : (l == 'i') ? "ï" : (l == 'o') ? "ö" : (l == 'u') ? "ü" : nullptr;
    else if (d == 0x4B) r = (l == 'c') ? "ç" : nullptr;
    return r ? std::string(r) : std::string(1, (char) l);
  }
  void poserMemo(const std::string& car, bool g, int p) { dernierCar = car; dernierGraph = g; dernierPoints = p; aDernier = true; poser(car, g, p); }
  void finCsi(byte b) {
    int n = atoi(csi.c_str()); if (n <= 0) n = 1;
    if (b == 0x41) { r = std::max(0, r - n); zoneIci(); }
    else if (b == 0x42) { r = std::min(23, r + n); zoneIci(); }
    else if (b == 0x43) { c = std::min(39, c + n); zoneIci(); }
    else if (b == 0x44) { c = std::max(0, c - n); zoneIci(); }
    else if (b == 0x4B) clearLineFromCursor();
    else if (b == 0x48) {  // CSI rangée ; colonne H
      int y = 1, x = 1; sscanf(csi.c_str(), "%d;%d", &y, &x);
      rangee0 = false;
      r = std::min(23, std::max(0, y - 1)); c = std::min(39, std::max(0, x - 1)); zoneIci();
    }
  }
  void writeByte(byte b) {
    switch (etat) {
      case 1:  // Après ESC
        etat = 0;
        if (b == 0x5B) { etat = 2; csi.clear(); return; }
        if (b == 0x39 || b == 0x3A || b == 0x3B) { saut = b - 0x38; etat = 8; return; }
        if (b == 0x23) { saut = 2; etat = 8; return; }
        attribut(b);
        return;
      case 2:  // Séquence CSI
        if (b >= 0x30 && b <= 0x3F) { csi += (char) b; return; }
        etat = 0; finCsi(b); return;
      case 3: memo = b; etat = 4; return;  // US : rangée
      case 4:  // US : colonne
        etat = 0;
        if (memo == 0x40) { if (!rangee0) { avant0r = r; avant0c = c; } rangee0 = true; return; }
        rangee0 = false;
        r = std::min(23, std::max(0, memo - 0x41)); c = std::min(39, std::max(0, b - 0x41)); zoneIci();
        return;
      case 5:  // SS2
        if (b == 0x41 || b == 0x42 || b == 0x43 || b == 0x48 || b == 0x4B) { memo = b; etat = 6; return; }
        etat = 0;
        if (g2simple(b)) poserMemo(g2simple(b), false, 0);
        return;
      case 6: etat = 0; poserMemo(g2compose(memo, b), false, 0); return;
      case 7:  // REP
        etat = 0;
        if (aDernier) { std::string car = dernierCar; bool g = dernierGraph; int p = dernierPoints; for (int i = 0; i < (b & 0x3F); i++) poser(car, g, p); }
        return;
      case 8: if (--saut <= 0) etat = 0; return;
    }
    switch (b) {
      case 0x1B: etat = 1; return;
      case 0x1F: etat = 3; return;
      case 0x19: etat = 5; return;
      case 0x0D: c = 0; zoneIci(); return;
      case 0x0A:
        if (rangee0) { rangee0 = false; r = avant0r; c = avant0c; zoneIci(); return; }
        descendre(); return;
      case 0x12: etat = 7; return;
      case 0x08: if (c > 0) c--; zoneIci(); return;
      case 0x09: c = std::min(39, c + 1); zoneIci(); return;
      case 0x0B: r = std::max(0, r - 1); zoneIci(); return;
      case 0x0C: effacer(); return;
      case 0x0E: graphicMode(); return;
      case 0x0F: textMode(); return;
      case 0x1E: r = 0; c = 0; zoneIci(); return;
      case 0x18: if (!rangee0) clearLineFromCursor(); return;
      case 0x11: g_curseur = true; return;
      case 0x14: g_curseur = false; return;
      case 0x07: g_bips++; return;
    }
    if (b < 0x20) return;
    if (modeGraphique) {
      if (b == 0x5F) { poserMemo(" ", true, 63); return; }  // Le pavé plein (Minitel1B_Hard l'envoie ainsi)
      if (b >= 0x40 && b < 0x60) return;
      int p = ((b & 1) ? 32 : 0) | ((b & 2) ? 16 : 0) | ((b & 4) ? 8 : 0) | ((b & 8) ? 4 : 0) | ((b & 16) ? 2 : 0) | ((b & 64) ? 1 : 0);
      poserMemo(" ", true, b == 0x7F ? 63 : p);
      return;
    }
    if (b == 0x7F) return;
    poserMemo(std::string(1, (char) b), false, 0);
  }
  void print(const char* s) {
    while (*s) {
      unsigned char c0 = *s;
      int n = (c0 >= 0xF0) ? 4 : (c0 >= 0xE0) ? 3 : (c0 >= 0xC0) ? 2 : 1;
      if (c0 < 0x20) { writeByte(c0); s++; continue; }
      poser(std::string(s, n), false, 0);
      s += n;
    }
  }
  String getString(unsigned long k) {  // Unicode -> UTF-8
    std::string s;
    if (k < 0x80) s += (char) k;
    else if (k < 0x800) { s += (char) (0xC0 | (k >> 6)); s += (char) (0x80 | (k & 0x3F)); }
    else { s += (char) (0xE0 | (k >> 12)); s += (char) (0x80 | ((k >> 6) & 0x3F)); s += (char) (0x80 | (k & 0x3F)); }
    return String(s);
  }
  unsigned long getKeyCode();
  void moveCursorXY(int x, int y) { c = x - 1; r = y - 1; zoneIci(); }
  void moveCursorUp(int n) { r -= n; if (r < 0) r = 0; zoneIci(); }
  void moveCursorRight(int n) { c += n; if (c > 39) c = 39; zoneIci(); }
  void moveCursorLeft(int n) { c -= n; if (c < 0) c = 0; zoneIci(); }
  void clearLineFromCursor() { for (int j = c; j < 40; j++) ecran[r][j] = Case(); }
  void cursor() { g_curseur = true; }
  void noCursor() { g_curseur = false; }
  void graphicMode() { modeGraphique = true; taille = 0; inverse = false; soulLatent = false; soulZone = false; }
  void textMode() { modeGraphique = false; }
  void graphic(byte p) { poser(" ", true, p); }
  void newScreen() { effacer(); }
  void echo(bool) { }
  void scrollMode() { }
  void pageMode() { }
  // Comme Minitel1B_Hard : l'attribut est envoyé, et la double hauteur descend le curseur
  void attributs(byte a) { writeByte(0x1B); writeByte(a); if (a == 0x4D || a == 0x4F) descendre(); }
  void bip() { g_bips++; }
};

// L'horloge : 14/11/2026 à 10:30:00 au lancement, puis le temps simulé
struct DateTime {
  int Y, M, D, h, m, s;
  int year() const { return Y; } int month() const { return M; } int day() const { return D; }
  int hour() const { return h; } int minute() const { return m; } int second() const { return s; }
};
struct RTC_DS3231 {
  DateTime now() { long t = 10 * 3600 + 30 * 60 + g_ms / 1000; return DateTime{2026, 11, 14, (int) (t / 3600) % 24, (int) (t / 60) % 60, (int) (t % 60)}; }
};

// La carte SD : un dossier de l'ordinateur
extern std::string g_dossierSD;
struct File {
  FILE* f = nullptr; std::string chemin; bool dossier = false; DIR* d = nullptr; std::string nom;
  explicit operator bool() const { return f != nullptr || d != nullptr; }
  int read() { if (!f) return -1; int c = fgetc(f); return c == EOF ? -1 : c; }
  int available() { if (!f) return 0; int c = fgetc(f); if (c == EOF) return 0; ungetc(c, f); return 1; }
  size_t write(uint8_t b) { if (!f) return 0; fputc(b, f); return 1; }
  void flush() { if (f) fflush(f); }
  void print(const char* s) { if (f) fputs(s, f); }
  void print(char c) { if (f) fputc(c, f); }
  void print(unsigned n) { if (f) fprintf(f, "%u", n); }
  void println(const char* s) { if (f) { fputs(s, f); fputs("\r\n", f); } }
  void close() { if (f) fclose(f); f = nullptr; if (d) closedir(d); d = nullptr; }
  bool isDirectory() { return dossier; }
  void rewindDirectory() { if (d) rewinddir(d); }
  const char* name() { return nom.c_str(); }
  unsigned long size() { struct stat st; stat(chemin.c_str(), &st); return st.st_size; }
  File openNextFile() {
    File r;
    if (!d) return r;
    struct dirent* e;
    while ((e = readdir(d))) {
      if (e->d_name[0] == '.') continue;
      r.chemin = chemin + "/" + e->d_name; r.nom = e->d_name;
      struct stat st; stat(r.chemin.c_str(), &st);
      if (S_ISDIR(st.st_mode)) { r.dossier = true; r.d = opendir(r.chemin.c_str()); }
      else r.f = fopen(r.chemin.c_str(), "rb");
      return r;
    }
    return r;
  }
};
struct SDClass {
  std::string complet(const char* n) { return g_dossierSD + n; }
  bool exists(const char* n) { struct stat st; return stat(complet(n).c_str(), &st) == 0; }
  bool remove(const char* n) { return ::remove(complet(n).c_str()) == 0; }
  File open(const char* n, int mode = FILE_READ) {
    File r; r.chemin = complet(n); r.nom = n;
    if (!strcmp(n, "/")) { r.dossier = true; r.chemin = g_dossierSD; r.d = opendir(g_dossierSD.c_str()); return r; }
    r.f = fopen(r.chemin.c_str(), mode == FILE_WRITE ? "ab+" : "rb");   // FILE_WRITE écrit à la fin, comme sur l'ATmega
    return r;
  }
};

// La liaison avec le PC (Serial, le câble USB) : une prise reliée à la passerelle
// (descripteur donné par la variable d'environnement REBASIC_PC), sinon rien n'est branché.
// Chaque attente d'un octet compte pour une milliseconde de temps simulé.
extern int g_fdPC;
struct SerialPC {
  int lu = -1;  // Octet lu d'avance par available()
  void begin(long) { }
  int available() {
    if (lu >= 0) return 1;
    if (g_fdPC >= 0) {
      pollfd p{g_fdPC, POLLIN, 0};
      unsigned char c;
      if (poll(&p, 1, 1) > 0 && ::read(g_fdPC, &c, 1) == 1) { lu = c; return 1; }
    }
    g_ms++;
    return 0;
  }
  int read() { if (!available()) return -1; int c = lu; lu = -1; return c; }
  size_t write(char c) { if (g_fdPC >= 0 && ::write(g_fdPC, &c, 1) != 1) return 0; return 1; }
};

extern SerialPC Serial;
extern SDClass SD;
extern Minitel minitel;
extern RTC_DS3231 rtc;
// L'horloge vue par reBASIC (Horloge.ino n'est pas compilé dans le simulateur)
extern boolean horlogeOK;
inline DateTime horlogeMaintenant() { return rtc.now(); }
inline boolean horlogeAttendrePC(unsigned long) { return horlogeOK; }
inline boolean horlogeDecoderPC(const char*) { return false; }
const byte BUZZER = 2;
const byte CHIP_SELECT = 53;
