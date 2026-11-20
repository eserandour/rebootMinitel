//////////////////////////////////////////////////////////////////////////////////////

// FICHIERS (menu 3) : la carte SD, et la copie avec le PC

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
               4 - Copier sur le PC (dans le dossier de la passerelle)
               SOMMAIRE revient à la liste.
    ENVOI, depuis la liste : les fichiers du PC (même présentation) ; une lettre choisit
               un fichier, ENVOI le copie sur la carte SD.

  La copie avec le PC passe par la passerelle (outils/passerelle/passerelle.py), comme
  SAVE "PC:NOM" dans reBASIC (voir « Fichiers sur le PC » dans Basic.ino). Une page
  Vidéotex contient des octets quelconques : ils voyagent en hexadécimal, 32 octets par
  ligne (64 caractères), que l'on demande un par un comme les lignes d'un LOAD :
    vers le PC   : -> @ECRIRE NOM.EXT  <- @OK  -> lignes hexadécimales, puis @FIN  <- @OK
    depuis le PC : -> @LIRE NOM.EXT    <- @OK (ou @ERR INTROUVABLE)
                   puis, pour chaque ligne : -> @SUITE  <- 64 caractères (ou moins), ou @FIN

  Comme FILES, on ne garde pas la liste en mémoire : on relit le dossier.
*/

const byte FICH_PAR_PAGE = 16;  // Fichiers par écran (lettres A à P)
#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
const byte FICH_NOM_MAX = 64;   // Noms longs possibles sur l'ESP32
#else
const byte FICH_NOM_MAX = 13;   // Noms 8.3 sur l'ATmega (12 caractères)
#endif

//////////////////////////////////////////////////////////////////////////////////////
// Affichage des nombres, sans objet String (pas d'allocation dans le tas, qui se
// morcelle vite dans les 8 Ko de mémoire vive de l'Arduino Mega)
//////////////////////////////////////////////////////////////////////////////////////

byte fichNombreEnTexte(unsigned long n, char* s)
{
  // Ecrit n en décimal dans s (11 octets au moins). Renvoie le nombre de chiffres.
  char tmp[10];
  byte i = 0;
  do {
    tmp[i++] = '0' + (n % 10);
    n /= 10;
  } while (n > 0);
  byte len = i;
  while (i > 0) *s++ = tmp[--i];
  *s = 0;
  return len;
}

//////////////////////////////////////////////////////////////////////////////////////

void fichAscii(const char* s)
{
  // Affiche un texte fait de chiffres et de signes simples (caractères du jeu G0)
  while (*s) minitel.printChar(*s++);
}

//////////////////////////////////////////////////////////////////////////////////////

void fichNombre(unsigned long n)
{
  char s[11];
  fichNombreEnTexte(n, s);
  fichAscii(s);
}

//////////////////////////////////////////////////////////////////////////////////////

void fichPages(int page, int pages)
{
  // « page/pages », aligné à droite de la première rangée (s'il y a plusieurs pages)
  if (pages <= 1) return;
  char p[24];
  byte n = fichNombreEnTexte(page + 1, p);
  p[n++] = '/';
  n += fichNombreEnTexte(pages, p + n);
  minitel.moveCursorXY(41 - n, 1);
  fichAscii(p);
}

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
  char t[11];
  byte n = fichNombreEnTexte(taille, t);
  minitel.moveCursorXY(41 - n, rangee);
  fichAscii(t);
}

//////////////////////////////////////////////////////////////////////////////////////

void fichListe(int page, int nombre)
{
  // Un écran de la liste
  int pages = (nombre + FICH_PAR_PAGE - 1) / FICH_PAR_PAGE;
  fichTitre(F(" Carte SD "));
  minitel.moveCursorXY(12, 1);
  fichNombre(nombre);
  minitel.print(nombre == 1 ? F(" fichier") : F(" fichiers"));
  fichPages(page, pages);
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
    if (strlen(nom) <= 26) minitel.print(nom);
    else {  // Nom long (ESP32) : on l'affiche coupé, sans couper une lettre accentuée
      char court[27];  // (nom sert encore à trouver le fichier suivant : on n'y touche pas)
      byte n = 25;
      while (n > 0 && (((byte) nom[n]) & 0xC0) == 0x80) n--;
      memcpy(court, nom, n);
      court[n] = '~';
      court[n + 1] = 0;
      minitel.print(court);
    }
    fichTaille(taille, rangee);
  }
  minitel.moveCursorXY(1, 21);
  minitel.print(F("Tapez la lettre d'un fichier."));
  minitel.moveCursorXY(1, 22);
  afficherTouche(F("ENVOI"));
  minitel.print(F(" : fichiers du PC"));
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
      minitel.print(ligne);
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
    minitel.print(nom);
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
    minitel.moveCursorXY(1, 15);
    minitel.print(F("4 - Copier sur le PC"));
    minitel.moveCursorXY(1, 24);
    afficherTouche(F("SOMMAIRE"));
    minitel.print(F(" : retour à la liste"));

    unsigned long k = 0;
    while (true) {
      k = minitel.getKeyCode();
      if (k == '4') {
        // On reste sur cet écran : le résultat de la copie s'affiche en dessous
        minitel.moveCursorXY(1, 17);
        minitel.clearLineFromCursor();
        minitel.moveCursorXY(1, 19);
        minitel.clearLineFromCursor();
        fichVersPC(chemin, taille);
        continue;
      }
      if (k == '1' || k == '3' || k == SOMMAIRE || (programme && k == '2')) break;
    }
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
      minitel.clearLineFromCursor();  // Efface le résultat d'une copie vers le PC
      minitel.moveCursorXY(1, 19);
      afficherTouche(F("ENVOI"));
      minitel.print(F(" : oui - "));
      afficherTouche(F("SOMMAIRE"));
      minitel.print(F(" : non"));
      minitel.clearLineFromCursor();
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
// Copie avec le PC
//////////////////////////////////////////////////////////////////////////////////////

#define PC_OCTETS_LIGNE 32  // Octets par ligne hexadécimale (64 caractères)

boolean fichNom83(const char* nom)
{
  // Le nom est-il accepté par le PC (format 8.3 : 1 à 8 caractères, extension de 1 à 3) ?
  byte n = 0, ext = 0;
  boolean point = false;
  for (const char* p = nom; *p; p++) {
    char c = *p;
    if (c == '.') {
      if (point || n == 0) return false;
      point = true;
      continue;
    }
    if (!(c >= 'A' && c <= 'Z') && !(c >= 'a' && c <= 'z') && !(c >= '0' && c <= '9') && c != '_' && c != '-') return false;
    if (point) ext++; else n++;
  }
  return n >= 1 && n <= 8 && (!point || (ext >= 1 && ext <= 3));
}

//////////////////////////////////////////////////////////////////////////////////////

void fichMessagePC(byte rangee)
{
  // Explique l'erreur d'un échange avec le PC (basErr), puis l'efface
  minitel.moveCursorXY(1, rangee);
  if (basErr == BAS_E_PC) minitel.print(F("PC absent : la passerelle tourne ?"));
  else if (basErr == BAS_E_INTROUV) minitel.print(F("Fichier introuvable sur le PC"));
  else minitel.print(F("Erreur de fichier"));
  minitel.clearLineFromCursor();
  basErr = 0;
  bipErreur();
}

//////////////////////////////////////////////////////////////////////////////////////

void fichProgres(unsigned long octets, unsigned long total, byte rangee)
{
  // Où en est la copie (n octets / total)
  minitel.moveCursorXY(1, rangee);
  fichNombre(octets);
  if (total > 0) {
    minitel.print(F(" / "));
    fichNombre(total);
  }
  minitel.print(F(" octets"));
}

//////////////////////////////////////////////////////////////////////////////////////

void fichVersPC(const char* chemin, unsigned long taille)
{
  // Copie un fichier de la carte SD dans le dossier du PC (résultat en rangées 17 et 19)
  minitel.moveCursorXY(1, 17);
  if (!fichNom83(chemin + 1)) {
    minitel.print(F("Nom trop long pour le PC (format 8.3)"));
    bipErreur();
    return;
  }
  File f = SD.open(chemin);
  if (!f) {
    minitel.print(F("Lecture impossible"));
    bipErreur();
    return;
  }
  minitel.print(F("Copie vers le PC..."));
  basErr = 0;
  if (!basPcBegin(PSTR("@ECRIRE"), chemin)) {
    f.close();
    fichMessagePC(19);
    return;
  }
  static const char HEX_[] PROGMEM = "0123456789ABCDEF";
  unsigned long envoyes = 0;
  byte n = 0;
  while (true) {
    int c = f.available() ? f.read() : -1;
    if (c >= 0) {
      Serial.write(pgm_read_byte(HEX_ + (c >> 4)));
      Serial.write(pgm_read_byte(HEX_ + (c & 15)));
      n++;
      envoyes++;
    }
    if (n == PC_OCTETS_LIGNE || (c < 0 && n > 0)) {
      Serial.write('\n');
      n = 0;
      if (envoyes % 1024 == 0) fichProgres(envoyes, taille, 19);
    }
    if (c < 0) break;
  }
  f.close();
  basPcSendP(PSTR("@FIN\n"));
  if (!basPcAnswer()) { fichMessagePC(19); return; }
  minitel.moveCursorXY(1, 17);
  minitel.print(F("Copié sur le PC : "));
  minitel.print(chemin + 1);
  minitel.clearLineFromCursor();
  minitel.moveCursorXY(1, 19);
  minitel.clearLineFromCursor();
  fichProgres(envoyes, 0, 19);
  bipOK();
}

//////////////////////////////////////////////////////////////////////////////////////

int fichHex(char c)
{
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  return -1;
}

//////////////////////////////////////////////////////////////////////////////////////

boolean fichDepuisPC(const char* nom, unsigned long taille, byte rangee)
{
  // Copie un fichier du PC sur la carte SD (le fichier de même nom est remplacé)
  char chemin[14];
  chemin[0] = '/';
  strncpy(chemin + 1, nom, 12);
  chemin[13] = 0;
  basErr = 0;
  if (!basPcBegin(PSTR("@LIRE"), chemin)) { fichMessagePC(rangee); return false; }
  if (SD.exists(chemin)) SD.remove(chemin);  // FILE_WRITE ajoute à la fin sur l'ATmega
  File f = SD.open(chemin, FILE_WRITE);
  if (!f) {
    minitel.moveCursorXY(1, rangee);
    minitel.print(F("Ecriture impossible sur la carte SD"));
    bipErreur();
    return false;
  }
  unsigned long recus = 0;
  byte octets[PC_OCTETS_LIGNE];
  while (basErr == 0 && basPcNext()) {
    byte n = 0;
    for (byte i = 0; basInBuf[i] && basInBuf[i + 1] && n < PC_OCTETS_LIGNE; i += 2) {
      int h = fichHex(basInBuf[i]), l = fichHex(basInBuf[i + 1]);
      if (h < 0 || l < 0) { basErr = BAS_E_FICHIER; break; }
      octets[n++] = (byte) (h * 16 + l);
    }
    if (basErr) break;
    if (f.write(octets, n) != n) { basErr = BAS_E_FICHIER; break; }
    recus += n;
    if (recus % 1024 == 0) fichProgres(recus, taille, rangee);
  }
  f.close();
  if (basErr) {
    SD.remove(chemin);  // Pas de fichier à moitié copié
    fichMessagePC(rangee);
    return false;
  }
  minitel.moveCursorXY(1, rangee);
  minitel.print(F("Copié sur la carte SD : "));
  minitel.print(nom);
  minitel.clearLineFromCursor();
  bipOK();
  return true;
}

//////////////////////////////////////////////////////////////////////////////////////

int pcListe(int page, char noms[][13], unsigned long* tailles)
{
  // Demande la liste des fichiers du PC ; garde ceux de la page (FICH_PAR_PAGE au plus).
  // Renvoie le nombre total de fichiers, ou -1 en cas d'erreur (basErr).
  basErr = 0;
  if (!basPcBegin(PSTR("@FILES"), NULL)) return -1;
  int nombre = 0;
  while (basPcNext()) {
    int i = nombre - page * FICH_PAR_PAGE;
    if (i >= 0 && i < FICH_PAR_PAGE) {
      char* espace = strchr(basInBuf, ' ');  // "NOM.EXT taille"
      tailles[i] = 0;
      if (espace != NULL) {
        *espace = 0;
        tailles[i] = strtoul(espace + 1, NULL, 10);
      }
      strncpy(noms[i], basInBuf, 12);
      noms[i][12] = 0;
    }
    nombre++;
  }
  return basErr ? -1 : nombre;
}

//////////////////////////////////////////////////////////////////////////////////////

void pcFichiers()
{
  // Les fichiers du dossier du PC : une lettre en choisit un, ENVOI le copie sur la carte
  int page = 0;
  char noms[FICH_PAR_PAGE][13];
  unsigned long tailles[FICH_PAR_PAGE];
  while (true) {
    fichTitre(F(" PC "));
    int nombre = pcListe(page, noms, tailles);
    if (nombre < 0) {
      minitel.moveCursorXY(1, 24);
      afficherTouche(F("SOMMAIRE"));
      minitel.print(F(" : retour à la carte SD"));
      fichMessagePC(4);
      while (minitel.getKeyCode() != SOMMAIRE);
      return;
    }
    int pages = (nombre + FICH_PAR_PAGE - 1) / FICH_PAR_PAGE;
    if (page > 0 && page >= pages) { page = 0; continue; }  // Le dossier a changé
    minitel.moveCursorXY(6, 1);
    fichNombre(nombre);
    minitel.print(nombre == 1 ? F(" fichier") : F(" fichiers"));
    fichPages(page, pages);
    if (nombre == 0) {
      minitel.moveCursorXY(1, 4);
      minitel.print(F("Le dossier du PC est vide."));
    }
    byte surPage = 0;
    for (byte i = 0; i < FICH_PAR_PAGE && page * FICH_PAR_PAGE + i < nombre; i++) {
      minitel.moveCursorXY(1, 3 + i);
      minitel.attributs(INVERSION_FOND);
      minitel.printChar('A' + i);
      minitel.attributs(FOND_NORMAL);
      minitel.print(F(" "));
      minitel.print(noms[i]);
      fichTaille(tailles[i], 3 + i);
      surPage++;
    }
    minitel.moveCursorXY(1, 21);
    minitel.print(F("Tapez la lettre d'un fichier."));
    if (pages > 1) {
      minitel.moveCursorXY(1, 23);
      afficherTouche(F("RETOUR"));
      minitel.print(F(" - "));
      afficherTouche(F("SUITE"));
      minitel.print(F(" : autres fichiers"));
    }
    minitel.moveCursorXY(1, 24);
    afficherTouche(F("SOMMAIRE"));
    minitel.print(F(" : retour à la carte SD"));

    int choix = -1;
    while (choix < 0) {
      unsigned long k = minitel.getKeyCode();
      if (k == SOMMAIRE) return;
      if (k == SUITE || k == RETOUR) {
        int nouvelle = page + ((k == SUITE) ? 1 : -1);
        if (nouvelle >= 0 && nouvelle < pages) { page = nouvelle; break; }
        minitel.bip();
      }
      if (k >= 'a' && k <= 'z') k -= 32;
      if (k >= 'A' && k < 'A' + FICH_PAR_PAGE) {
        if (k - 'A' < surPage) choix = k - 'A';
        else minitel.bip();
      }
    }
    if (choix < 0) continue;  // Changement de page

    // Confirmation, puis copie (rangées 17 à 21 : sous la liste, qui en occupe 3 à 18)
    for (byte r = 19; r <= 24; r++) {
      minitel.moveCursorXY(1, r);
      minitel.clearLineFromCursor();
    }
    char chemin[14];
    chemin[0] = '/';
    strcpy(chemin + 1, noms[choix]);
    minitel.moveCursorXY(1, 20);
    minitel.print(F("Copier "));
    minitel.print(noms[choix]);
    minitel.print(F(" sur la carte ?"));
    if (SD.exists(chemin)) {
      minitel.moveCursorXY(1, 21);
      minitel.print(F("(remplace celui de la carte)"));
    }
    minitel.moveCursorXY(1, 23);
    afficherTouche(F("ENVOI"));
    minitel.print(F(" : oui - "));
    afficherTouche(F("SOMMAIRE"));
    minitel.print(F(" : non"));
    unsigned long r = 0;
    while (r != ENVOI && r != SOMMAIRE) r = minitel.getKeyCode();
    if (r == SOMMAIRE) continue;
    for (byte rr = 20; rr <= 23; rr++) {
      minitel.moveCursorXY(1, rr);
      minitel.clearLineFromCursor();
    }
    minitel.moveCursorXY(1, 20);
    minitel.print(F("Copie sur la carte SD..."));
    fichDepuisPC(noms[choix], tailles[choix], 21);
    minitel.moveCursorXY(1, 24);
    minitel.clearLineFromCursor();
    afficherTouche(F("ENVOI"));
    minitel.print(F(" : suite"));
    while (true) {
      unsigned long k = minitel.getKeyCode();
      if (k == ENVOI || k == SOMMAIRE) break;
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
      if (k == ENVOI) {
        pcFichiers();
        break;  // Retour à la liste de la carte, relue : un fichier a pu arriver
      }
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
