//////////////////////////////////////////////////////////////////////////////////////
// Simulateur de reBASIC
// Usage : simulateur dossierSD fichierTouches fichierEcran [limite] [--interactif]
//////////////////////////////////////////////////////////////////////////////////////
#include "imitations.h"

std::vector<Touche> g_touches;
size_t g_prochaine = 0;
bool g_curseur = false;
std::string g_dossierSD;
std::string g_journal;
std::string g_fichierEcran;
unsigned long g_ms = 0;
int g_erreurs = 0;       // Erreurs affichées par reBASIC (compteur ajouté dans basPrintError)
int g_interruptions = 0; // ANNULATION envoyées par le simulateur pour arrêter une boucle sans fin
int g_attente = 0;
int g_bips = 0;
bool g_capturee = false;
bool g_interactif = false;
long g_limite = 400000;  // Lectures du clavier sans touche avant d'interrompre le programme

SDClass SD;
int g_fdPC = getenv("REBASIC_PC") ? atoi(getenv("REBASIC_PC")) : -1;
SerialPC Serial;
Minitel minitel;
RTC_DS3231 rtc;
boolean horlogeOK = true;

#include "prototypes.h"
#include "Basic.cpp"

//////////////////////////////////////////////////////////////////////////////////////

void ajouterTouches(const std::string& t)
{
  // Texte normal : tapé quand reBASIC attend une ligne. Retour à la ligne : ENVOI.
  // {ENVOI} {ANNUL} {SOMM} {SUITE} {RETOUR} {GUIDE} {CORR} : touches de fonction
  // {k:x} : la touche x, frappée même pendant l'exécution (pour INKEY$)
  // {WAIT:n} : n millisecondes sans toucher au clavier
  // {CLR} : efface l'écran simulé   {SNAP} : capture l'écran à cet instant
  for (size_t i = 0; i < t.size(); ) {
    unsigned char c = t[i];
    if (c == '{') {
      size_t j = t.find('}', i);
      std::string m = t.substr(i + 1, j - i - 1);
      i = j + 1;
      Touche k{0, true, 0};
      if (m == "ANNUL") k.code = ANNULATION;
      else if (m == "SOMM") k.code = SOMMAIRE;
      else if (m == "SUITE") k.code = SUITE;
      else if (m == "RETOUR") k.code = RETOUR;
      else if (m == "ENVOI") k.code = ENVOI;
      else if (m == "GUIDE") { k.code = GUIDE; k.toujours = false; }      // Tapées pendant une saisie
      else if (m == "CORR") { k.code = CORRECTION; k.toujours = false; }
      else if (m == "CLR") k.special = 1;
      else if (m == "SNAP") k.special = 2;
      else if (m.rfind("WAIT:", 0) == 0) { k.special = 3; k.code = atol(m.c_str() + 5); }
      else if (m.rfind("k:", 0) == 0) k.code = (unsigned char) m[2];
      else continue;
      g_touches.push_back(k);
      continue;
    }
    if (c == '\r') { i++; continue; }
    if (c == '\n') { g_touches.push_back({0x0D, false, 0}); i++; continue; }
    if (c >= 0xE0 && i + 2 < t.size()) {  // Caractère sur 3 octets en UTF-8 (flèches...) : son code Unicode, comme getKeyCode
      g_touches.push_back({(unsigned long) (((c & 0x0F) << 12) | (((unsigned char) t[i + 1] & 0x3F) << 6) | ((unsigned char) t[i + 2] & 0x3F)), false, 0});
      i += 3;
      continue;
    }
    if (c >= 0xC0 && i + 1 < t.size()) {  // Lettre accentuée (2 octets en UTF-8) : son code Unicode
      g_touches.push_back({(unsigned long) (((c & 0x1F) << 6) | ((unsigned char) t[i + 1] & 0x3F)), false, 0});
      i += 2;
      continue;
    }
    g_touches.push_back({c, false, 0});
    i++;
  }
}

//////////////////////////////////////////////////////////////////////////////////////

std::string sextant(int p)
{
  // Caractère Unicode « sextant » pour une case semi-graphique (2 x 3 points)
  int v = ((p & 32) ? 1 : 0) | ((p & 16) ? 2 : 0) | ((p & 8) ? 4 : 0) | ((p & 4) ? 8 : 0) | ((p & 2) ? 16 : 0) | ((p & 1) ? 32 : 0);
  if (v == 0) return " ";
  if (v == 63) return "█";
  if (v == 21) return "▌";
  if (v == 42) return "▐";
  unsigned cp = 0x1FB00 + v - 1 - (v > 21 ? 1 : 0) - (v > 42 ? 1 : 0);
  char b[5] = {(char) (0xF0 | (cp >> 18)), (char) (0x80 | ((cp >> 12) & 0x3F)), (char) (0x80 | ((cp >> 6) & 0x3F)), (char) (0x80 | (cp & 0x3F)), 0};
  return b;
}

//////////////////////////////////////////////////////////////////////////////////////

void afficherTerminal()
{
  // Mode interactif : dessine l'écran du Minitel dans le terminal (couleurs ANSI)
  static const int gris[8] = {236, 244, 250, 254, 242, 246, 252, 255};  // Niveaux de gris du Minitel 1B
  std::string s = "\033[H\033[2J┌";
  for (int j = 0; j < 40; j++) s += "─";
  s += "┐\n";
  for (int i = 0; i < 24; i++) {
    s += "│";
    for (int j = 0; j < 40; j++) {
      Case& x = minitel.ecran[i][j];
      char col[64];
      snprintf(col, sizeof col, "\033[38;5;%dm\033[48;5;%dm%s%s%s", gris[x.couleur & 7], x.fond ? gris[x.fond & 7] : 16,
               x.inverse ? "\033[7m" : "", x.souligne ? "\033[4m" : "", x.clignote ? "\033[5m" : "");
      s += col;
      s += x.couverte ? std::string(" ") : x.graphique ? sextant(x.points) : x.car;
      s += "\033[0m";
    }
    s += "│\n";
  }
  s += "└";
  for (int j = 0; j < 40; j++) s += "─";
  s += "┘\n";
  fputs(s.c_str(), stdout);
  fputs("Tapez une ligne puis Entrée. {ANNUL}, {GUIDE}, {SUITE}... pour les touches spéciales,\n"
        "{SOMM} ou Ctrl-D pour quitter.\n> ", stdout);
  fflush(stdout);
}

//////////////////////////////////////////////////////////////////////////////////////

void capture(bool forcer)
{
  // Enregistre l'état de l'écran (une ligne par case) pour ecran.py
  if (g_fichierEcran.empty()) return;
  if (g_capturee && !forcer) return;
  g_capturee = true;
  FILE* f = fopen(g_fichierEcran.c_str(), "w");
  for (int i = 0; i < 24; i++) {
    for (int j = 0; j < 40; j++) {
      Case& x = minitel.ecran[i][j];
      fprintf(f, "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%s\n", i, j, x.couleur, x.inverse ? 1 : 0, x.graphique ? 1 : 0, x.points,
              x.fond, x.souligne ? 1 : 0, x.clignote ? 1 : 0, x.taille, x.couverte ? 1 : 0, x.car.c_str());
    }
  }
  fprintf(f, "CURSEUR\t%d\t%d\nBIPS\t%d\nJOURNAL\t%s\n", minitel.r, minitel.c, g_bips, g_journal.c_str());
  fclose(f);
}

//////////////////////////////////////////////////////////////////////////////////////

unsigned long Minitel::getKeyCode()
{
  g_ms++;
  while (g_prochaine < g_touches.size()) {
    Touche& k = g_touches[g_prochaine];
    if (k.special == 1) { minitel.effacer(); g_prochaine++; continue; }
    if (k.special == 2) { capture(true); g_prochaine++; continue; }
    if (k.special == 3) {
      if (g_attente < (long) k.code) { g_attente++; return 0; }
      g_attente = 0; g_prochaine++; continue;
    }
    break;
  }
  if (g_prochaine < g_touches.size()) {
    Touche& k = g_touches[g_prochaine];
    if (k.toujours || g_curseur) { g_prochaine++; g_attente = 0; return k.code; }
  }
  else if (g_curseur) {  // reBASIC attend une ligne et le script est fini
    if (!g_interactif) { capture(false); return SOMMAIRE; }
    afficherTerminal();
    char ligne[512];
    if (!fgets(ligne, sizeof ligne, stdin)) { fputs("\n", stdout); return SOMMAIRE; }
    ajouterTouches(ligne);
    return 0;
  }
  if (++g_attente > g_limite) {  // Boucle sans fin : on interrompt le programme
    capture(false);
    g_attente = 0;
    g_interruptions++;
    return ANNULATION;
  }
  return 0;
}

//////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
  if (argc < 4) {
    fprintf(stderr, "Usage : %s dossierSD fichierTouches fichierEcran [limite] [--interactif]\n", argv[0]);
    return 1;
  }
  g_dossierSD = argv[1];
  if (!g_dossierSD.empty() && g_dossierSD.back() == '/') g_dossierSD.pop_back();
  g_fichierEcran = argv[3];
  for (int i = 4; i < argc; i++) {
    if (!strcmp(argv[i], "--interactif")) g_interactif = true;
    else g_limite = atol(argv[i]);
  }
  std::string tout;
  if (strcmp(argv[2], "-") != 0) {
    FILE* f = fopen(argv[2], "rb");
    if (!f) { fprintf(stderr, "Fichier introuvable : %s\n", argv[2]); return 1; }
    char b[4096]; size_t n;
    while ((n = fread(b, 1, sizeof b, f)) > 0) tout.append(b, n);
    fclose(f);
  }
  ajouterTouches(tout);
  basic();
  capture(false);
  if (!g_fichierEcran.empty()) {
    FILE* e = fopen((g_fichierEcran + ".err").c_str(), "w");
    fprintf(e, "%d %d\n", g_erreurs, g_interruptions);
    fclose(e);
  }
  return 0;
}
