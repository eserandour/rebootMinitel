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
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <strings.h>

typedef unsigned char byte;
typedef bool boolean;

// Mémoire programme (PROGMEM) : sur PC, tout est en mémoire vive
#define PSTR(s) (s)
#define strncpy_P strncpy
#define strcpy_P strcpy
#define strlen_P strlen
#define strncmp_P strncmp
#define strcasecmp_P strcasecmp
#define pgm_read_byte(p) (*(const unsigned char*)(p))

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
struct Case { std::string car = " "; int couleur = 7; bool inverse = false; bool graphique = false; int points = 0; };

// Une touche du script de clavier
struct Touche { unsigned long code; bool toujours; int special; };
extern std::vector<Touche> g_touches;
extern size_t g_prochaine;
extern bool g_curseur;
extern int g_bips;
void capture(bool forcer);

// Le Minitel : un écran de 40 x 24 cases en mode rouleau
struct Minitel {
  Case ecran[24][40];
  int r = 0, c = 0, couleur = 7;
  bool inverse = false, modeGraphique = false;
  void effacer() { for (auto& l : ecran) for (auto& x : l) x = Case(); r = c = 0; couleur = 7; inverse = false; modeGraphique = false; }
  void defiler() {
    for (int i = 0; i < 23; i++) for (int j = 0; j < 40; j++) ecran[i][j] = ecran[i + 1][j];
    for (int j = 0; j < 40; j++) ecran[23][j] = Case();
    r = 23;
  }
  void poser(const std::string& car, bool graphique, int points) {
    Case& x = ecran[r][c];
    x.car = car; x.couleur = couleur; x.inverse = graphique ? false : inverse; x.graphique = graphique; x.points = points;
    if (++c >= 40) { c = 0; if (++r >= 24) defiler(); }
  }
  void writeByte(byte b) {
    if (b == 0x0D) { c = 0; return; }
    if (b == 0x0A) { if (++r >= 24) defiler(); return; }
    if (b == 0x08) { if (c > 0) c--; return; }
    if (b == 0x0C) { effacer(); return; }
    if (b == 0x0E) { modeGraphique = true; return; }
    if (b == 0x0F) { modeGraphique = false; return; }
    if (b < 0x20) return;
    poser(std::string(1, (char) b), false, 0);
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
  String getString(unsigned long k) { std::string s; s += (char) (k >> 8); s += (char) (k & 0xFF); return String(s); }
  unsigned long getKeyCode();
  void moveCursorXY(int x, int y) { c = x - 1; r = y - 1; }
  void moveCursorUp(int n) { r -= n; if (r < 0) r = 0; }
  void moveCursorRight(int n) { c += n; if (c > 39) c = 39; }
  void moveCursorLeft(int n) { c -= n; if (c < 0) c = 0; }
  void clearLineFromCursor() { for (int j = c; j < 40; j++) ecran[r][j] = Case(); }
  void cursor() { g_curseur = true; }
  void noCursor() { g_curseur = false; }
  void graphicMode() { modeGraphique = true; }
  void textMode() { modeGraphique = false; }
  void graphic(byte p) { poser(" ", true, p); }
  void newScreen() { effacer(); }
  void echo(bool) { }
  void scrollMode() { }
  void pageMode() { }
  void attributs(byte a) {
    if (a >= 0x40 && a <= 0x47) couleur = a - 0x40;
    else if (a == INVERSION_FOND) inverse = true;
    else if (a == FOND_NORMAL) inverse = false;
  }
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

extern SDClass SD;
extern Minitel minitel;
extern RTC_DS3231 rtc;
const byte BUZZER = 2;
const byte CHIP_SELECT = 53;
