//////////////////////////////////////////////////////////////////////////////////////

// CARTE SD (menu 3)

//////////////////////////////////////////////////////////////////////////////////////
/*
  Le gestionnaire de la carte SD : la liste des fichiers du dossier racine, par ordre
  alphabétique, FICH_PAR_PAGE par écran, chacun repéré par une lettre.

    Liste :    une lettre choisit le fichier ; SUITE et RETOUR changent de page ;
               SOMMAIRE revient au menu.
    Fichier :  1 - Afficher (une page Vidéotex .VDT est dessinée ; un autre fichier est
                   affiché comme du texte, écran par écran)
               2 - Lancer (programme .BAS : il est chargé dans reBASIC, puis lancé)
               3 - Supprimer (après confirmation par ENVOI)
               SOMMAIRE revient à la liste.

  Comme FILES, on ne garde pas la liste en mémoire : on relit le dossier.
*/

const byte FICH_PAR_PAGE = 16;  // Fichiers par écran (lettres A à P)
#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
const byte FICH_NOM_MAX = 64;   // Noms longs possibles sur l'ESP32
#else
const byte FICH_NOM_MAX = 13;   // Noms 8.3 sur l'ATmega (12 caractères)
#endif

//////////////////////////////////////////////////////////////////////////////////////

boolean fichSuivant(const char* courant, char* resultat, unsigned long* taille)
{
  // Le fichier dont le nom vient juste après courant par ordre alphabétique (le premier si
  // courant est vide). Renvoie false s'il n'y en a pas. Les noms trop longs sont ignorés.
  File racine = SD.open("/");
  if (!racine) return false;
  boolean trouve = false;
  racine.rewindDirectory();
  while (true) {
    File f = racine.openNextFile();
    if (!f) break;
    if (!f.isDirectory()) {
      const char* nom = f.name();
      if (nom[0] == '/') nom++;
      if (strlen(nom) < FICH_NOM_MAX && (courant[0] == 0 || strcmp(nom, courant) > 0) &&
          (!trouve || strcmp(nom, resultat) < 0)) {
        strcpy(resultat, nom);
        *taille = f.size();
        trouve = true;
      }
    }
    f.close();
  }
  racine.close();
  return trouve;
}

//////////////////////////////////////////////////////////////////////////////////////

int fichNombre()
{
  // Nombre de fichiers (de la liste)
  char nom[FICH_NOM_MAX], suivant[FICH_NOM_MAX];
  unsigned long taille;
  nom[0] = 0;
  int n = 0;
  while (fichSuivant(nom, suivant, &taille)) {
    strcpy(nom, suivant);
    n++;
  }
  return n;
}

//////////////////////////////////////////////////////////////////////////////////////

boolean fichNumero(int numero, char* nom, unsigned long* taille)
{
  // Le fichier numéro (0 pour le premier) de la liste
  char suivant[FICH_NOM_MAX];
  nom[0] = 0;
  for (int i = 0; i <= numero; i++) {
    if (!fichSuivant(nom, suivant, taille)) return false;
    strcpy(nom, suivant);
  }
  return true;
}

//////////////////////////////////////////////////////////////////////////////////////

boolean fichExtension(const char* nom, const char* extension)
{
  // Le nom finit-il par cette extension (".BAS", ".VDT"..., en mémoire programme) ?
  byte n = strlen(nom);
  byte e = strlen_P(extension);
  return n > e && strcasecmp_P(nom + n - e, extension) == 0;
}

//////////////////////////////////////////////////////////////////////////////////////

void fichTitre(const __FlashStringHelper* titre)
{
  minitel.newScreen();
  minitel.pageMode();
  minitel.noCursor();
  minitel.moveCursorXY(1, 1);
  minitel.attributs(INVERSION_FOND);
  minitel.print(titre);
  minitel.attributs(FOND_NORMAL);
}

//////////////////////////////////////////////////////////////////////////////////////

void fichTaille(unsigned long taille, byte rangee)
{
  // La taille, alignée à droite de l'écran
  String t = String(taille);
  minitel.moveCursorXY(41 - t.length(), rangee);
  minitel.print(t);
}

//////////////////////////////////////////////////////////////////////////////////////

void fichListe(int page, int nombre)
{
  // Un écran de la liste
  int pages = (nombre + FICH_PAR_PAGE - 1) / FICH_PAR_PAGE;
  fichTitre(F(" Carte SD "));
  minitel.moveCursorXY(12, 1);
  minitel.print(String(nombre));
  minitel.print(nombre == 1 ? F(" fichier") : F(" fichiers"));
  if (pages > 1) {
    String p = String(page + 1) + "/" + String(pages);
    minitel.moveCursorXY(41 - p.length(), 1);
    minitel.print(p);
  }
  if (nombre == 0) {
    minitel.moveCursorXY(1, 4);
    minitel.print(F("La carte SD est vide, absente ou"));
    minitel.moveCursorXY(1, 5);
    minitel.print(F("illisible."));
  }
  char nom[FICH_NOM_MAX], suivant[FICH_NOM_MAX];
  unsigned long taille;
  if (page > 0) fichNumero(page * FICH_PAR_PAGE - 1, nom, &taille);
  else nom[0] = 0;
  for (byte i = 0; i < FICH_PAR_PAGE && fichSuivant(nom, suivant, &taille); i++) {
    strcpy(nom, suivant);
    byte rangee = 3 + i;
    minitel.moveCursorXY(1, rangee);
    minitel.attributs(INVERSION_FOND);
    minitel.printChar('A' + i);
    minitel.attributs(FOND_NORMAL);
    minitel.print(F(" "));
    String n = nom;
    if (n.length() > 26) n = n.substring(0, 25) + "~";  // Nom long (ESP32) : on le coupe
    minitel.print(n);
    fichTaille(taille, rangee);
  }
  minitel.moveCursorXY(1, 21);
  minitel.print(F("Tapez la lettre d'un fichier."));
  minitel.moveCursorXY(1, 23);
  if (pages > 1) {
    afficherTouche(F("RETOUR"));
    minitel.print(F(" - "));
    afficherTouche(F("SUITE"));
    minitel.print(F(" : autres fichiers"));
  }
  minitel.moveCursorXY(1, 24);
  afficherTouche(F("SOMMAIRE"));
  minitel.print(F(" : retour au menu"));
}

//////////////////////////////////////////////////////////////////////////////////////

boolean fichAttendreSuite()
{
  // Texte affiché écran par écran : SUITE continue, SOMMAIRE arrête (renvoie false)
  minitel.moveCursorXY(1, 24);
  afficherTouche(F("SUITE"));
  minitel.print(F(" : suite - "));
  afficherTouche(F("SOMMAIRE"));
  minitel.print(F(" : retour"));
  unsigned long k = 0;
  while (k != SUITE && k != ENVOI && k != SOMMAIRE) k = minitel.getKeyCode();
  return k != SOMMAIRE;
}

//////////////////////////////////////////////////////////////////////////////////////

void fichAfficherTexte(const char* chemin)
{
  // Le fichier comme du texte, 22 rangées par écran. Les caractères accentués (UTF-8) sont
  // envoyés tels quels à minitel.print() ; les autres caractères de commande sont ignorés.
  File f = SD.open(chemin);
  if (!f) return;
  minitel.newScreen();
  minitel.pageMode();
  char ligne[4 * 40 + 1];  // Une rangée : 40 caractères, de 1 à 4 octets chacun
  byte octets = 0, colonnes = 0, rangee = 1;
  boolean fin = false;
  while (!fin) {
    int c = f.available() ? f.read() : -1;
    boolean couper = false;
    if (c < 0) { fin = true; couper = (octets > 0); }
    else if (c == '\n') couper = true;
    else if (c == '\t') { ligne[octets++] = ' '; colonnes++; }
    else if (c >= 0x20) {
      ligne[octets++] = (char) c;
      if ((c & 0xC0) != 0x80) colonnes++;  // Un octet de suite UTF-8 ne compte pas
    }
    if (colonnes == 40 && c >= 0) {
      // Rangée pleine : on attend la fin d'un caractère accentué avant de couper
      int p = f.peek();
      if (p < 0 || (p & 0xC0) != 0x80) {
        couper = true;
        // Une ligne de 40 caractères pile : sa fin de ligne ne doit pas laisser une rangée vide
        if (p == '\r') { f.read(); p = f.peek(); }
        if (p == '\n') f.read();
      }
    }
    if (couper) {
      ligne[octets] = 0;
      minitel.moveCursorXY(1, rangee);
      minitel.print(String(ligne));
      octets = 0;
      colonnes = 0;
      if (++rangee > 22 && !fin) {
        if (!fichAttendreSuite()) { f.close(); return; }
        minitel.newScreen();
        rangee = 1;
      }
    }
  }
  f.close();
  minitel.moveCursorXY(1, 24);
  afficherTouche(F("SOMMAIRE"));
  minitel.print(F(" : retour (fin du fichier)"));
  while (minitel.getKeyCode() != SOMMAIRE);
}

//////////////////////////////////////////////////////////////////////////////////////

byte fichFichier(const char* nom, unsigned long taille)
{
  // L'écran d'un fichier. Renvoie 0 pour revenir à la liste, 1 si le fichier a été
  // supprimé, 2 si un programme a été lancé (on revient alors au menu).
  char chemin[FICH_NOM_MAX + 1];
  chemin[0] = '/';
  strcpy(chemin + 1, nom);
  boolean programme = fichExtension(nom, PSTR(".BAS"));
  boolean page = fichExtension(nom, PSTR(".VDT"));
  while (true) {
    fichTitre(F(" Carte SD "));
    minitel.moveCursorXY(1, 4);
    minitel.print(String(nom));
    fichTaille(taille, 4);
    minitel.moveCursorXY(1, 5);
    if (programme) minitel.print(F("Programme reBASIC"));
    else if (page) minitel.print(F("Page Vidéotex"));
    else minitel.print(F("Fichier"));
    minitel.moveCursorXY(1, 9);
    minitel.print(F("1 - Afficher"));
    if (programme) {
      minitel.moveCursorXY(1, 11);
      minitel.print(F("2 - Lancer"));
    }
    minitel.moveCursorXY(1, 13);
    minitel.print(F("3 - Supprimer"));
    minitel.moveCursorXY(1, 24);
    afficherTouche(F("SOMMAIRE"));
    minitel.print(F(" : retour à la liste"));

    unsigned long k = 0;
    while (k != '1' && k != '3' && k != SOMMAIRE && !(programme && k == '2')) k = minitel.getKeyCode();
    if (k == SOMMAIRE) return 0;
    if (k == '1') {
      if (page) {
        vdtAfficher(nom);
        while (minitel.getKeyCode() != SOMMAIRE);
        minitel.newScreen();
        vdtEffacerRangee0();
        minitel.textMode();
      }
      else fichAfficherTexte(chemin);
    }
    else if (k == '2') {
      // Le programme est chargé puis lancé par reBASIC (LOAD et RUN, voir basic())
      strncpy(basLancer, nom, BAS_STR_MAX);
      basLancer[BAS_STR_MAX] = 0;
      basic();
      return 2;
    }
    else {
      minitel.moveCursorXY(1, 17);
      minitel.print(F("Supprimer ce fichier ?"));
      minitel.moveCursorXY(1, 19);
      afficherTouche(F("ENVOI"));
      minitel.print(F(" : oui - "));
      afficherTouche(F("SOMMAIRE"));
      minitel.print(F(" : non"));
      unsigned long r = 0;
      while (r != ENVOI && r != SOMMAIRE) r = minitel.getKeyCode();
      if (r == ENVOI) {
        if (SD.remove(chemin)) { bipOK(); return 1; }
        bipErreur();
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////////////////////

void carteSD()
{
  int page = 0;
  while (true) {
    int nombre = fichNombre();
    int pages = (nombre + FICH_PAR_PAGE - 1) / FICH_PAR_PAGE;
    if (page >= pages) page = (pages > 0) ? pages - 1 : 0;
    fichListe(page, nombre);
    unsigned long k = 0;
    int choix = -1;
    while (choix < 0) {
      k = minitel.getKeyCode();
      if (k == SOMMAIRE) return;
      if (k == SUITE || k == RETOUR) {
        int nouvelle = page + ((k == SUITE) ? 1 : -1);
        if (nouvelle >= 0 && nouvelle < pages) { page = nouvelle; break; }
        minitel.bip();
      }
      if (k >= 'a' && k <= 'z') k -= 32;  // Minuscule : comme la majuscule
      if (k >= 'A' && k < 'A' + FICH_PAR_PAGE) {
        int i = page * FICH_PAR_PAGE + (k - 'A');
        if (i < nombre) choix = i;
        else minitel.bip();
      }
    }
    if (choix < 0) continue;  // Changement de page
    char nom[FICH_NOM_MAX];
    unsigned long taille;
    if (!fichNumero(choix, nom, &taille)) continue;
    if (fichFichier(nom, taille) == 2) return;  // Un programme a été lancé
  }
}

//////////////////////////////////////////////////////////////////////////////////////
