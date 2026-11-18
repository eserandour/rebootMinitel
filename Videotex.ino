//////////////////////////////////////////////////////////////////////////////////////

// PAGES VIDÉOTEX (menu 2)

//////////////////////////////////////////////////////////////////////////////////////
/*
  Affiche les pages Vidéotex de la carte SD : les fichiers .VDT du dossier racine, par
  ordre alphabétique. Un fichier .VDT contient les octets d'une page Minitel, envoyés
  tels quels au Minitel (on en trouve en ligne, ou on les compose avec un éditeur comme
  MiEdit).

    SUITE    : page suivante (après la dernière, on revient à la première)
    RETOUR   : page précédente
    ENVOI    : défilement automatique (une page toutes les VDT_DUREE millisecondes),
               ou arrêt du défilement
    SOMMAIRE : retour au menu (aussi pendant l'envoi d'une page)

  Pour ne pas garder la liste des pages en mémoire, on relit le dossier à chaque
  changement de page, comme FILES dans reBASIC.
*/

const unsigned long VDT_DUREE = 10000;  // Défilement automatique : 10 secondes par page
#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
const byte VDT_NOM_MAX = 64;  // Noms longs possibles sur l'ESP32
#else
const byte VDT_NOM_MAX = 13;  // Noms 8.3 sur l'ATmega (12 caractères)
#endif

//////////////////////////////////////////////////////////////////////////////////////

boolean vdtEstUnePage(const char* nom)
{
  byte n = strlen(nom);
  return n > 4 && n < VDT_NOM_MAX && strcasecmp_P(nom + n - 4, PSTR(".VDT")) == 0;
}

//////////////////////////////////////////////////////////////////////////////////////

boolean vdtVoisine(const char* courant, int sens, char* resultat)
{
  // Cherche la page qui suit (sens > 0) ou qui précède (sens < 0) la page courant, par
  // ordre alphabétique. Si courant est vide : la première page (sens > 0) ou la dernière.
  // Renvoie false s'il n'y en a pas.
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
      if (vdtEstUnePage(nom)) {
        boolean candidat = courant[0] == 0 ||
                           (sens > 0 ? strcmp(nom, courant) > 0 : strcmp(nom, courant) < 0);
        boolean meilleur = !trouve ||
                           (sens > 0 ? strcmp(nom, resultat) < 0 : strcmp(nom, resultat) > 0);
        if (candidat && meilleur) {
          strcpy(resultat, nom);
          trouve = true;
        }
      }
    }
    f.close();
  }
  racine.close();
  return trouve;
}

//////////////////////////////////////////////////////////////////////////////////////

int vdtNombreDePages()
{
  int n = 0;
  File racine = SD.open("/");
  if (!racine) return 0;
  racine.rewindDirectory();
  while (true) {
    File f = racine.openNextFile();
    if (!f) break;
    if (!f.isDirectory()) {
      const char* nom = f.name();
      if (nom[0] == '/') nom++;
      if (vdtEstUnePage(nom)) n++;
    }
    f.close();
  }
  racine.close();
  return n;
}

//////////////////////////////////////////////////////////////////////////////////////

void vdtEffacerRangee0()
{
  // La rangée 0 (ligne de service, tout en haut) n'est pas effacée par newScreen() : une
  // page peut y avoir écrit. US 0x40 0x41 y va, CAN l'efface, LF revient à l'écran.
  minitel.writeByte(0x1F);
  minitel.writeByte(0x40);
  minitel.writeByte(0x41);
  minitel.writeByte(0x18);
  minitel.writeByte(0x0A);
}

//////////////////////////////////////////////////////////////////////////////////////

unsigned long vdtAfficher(const char* nom)
{
  // Envoie la page au Minitel. Renvoie 0, ou la touche de fonction frappée pendant
  // l'envoi (SOMMAIRE, SUITE, RETOUR ou ENVOI) : on arrête alors l'envoi.
  char chemin[VDT_NOM_MAX + 1];
  chemin[0] = '/';
  strcpy(chemin + 1, nom);
  minitel.newScreen();
  minitel.pageMode();  // Une page Vidéotex est faite pour le mode page : pas de défilement
  vdtEffacerRangee0();  // Ce que la page précédente y a écrit
  File f = SD.open(chemin);
  if (!f) return 0;
  unsigned long touche = 0;
  int compteur = 0;
  while (f.available()) {
    minitel.writeByte(f.read() & 0x7F);  // Le Vidéotex est un code sur 7 bits
    if (++compteur == 32) {  // De temps en temps, on regarde le clavier
      compteur = 0;
      unsigned long k = minitel.getKeyCode();
      if (k == SOMMAIRE || k == SUITE || k == RETOUR || k == ENVOI) { touche = k; break; }
    }
  }
  f.close();
  minitel.noCursor();  // Une page peut avoir allumé le curseur
  return touche;
}

//////////////////////////////////////////////////////////////////////////////////////

void vdtEcranPresentation(int n)
{
  // L'écran qui précède les pages : leur nombre et les touches
  minitel.newScreen();
  minitel.pageMode();
  minitel.noCursor();
  minitel.moveCursorXY(1, 3);
  minitel.attributs(INVERSION_FOND);
  minitel.print(F(" Pages Vidéotex "));
  minitel.attributs(FOND_NORMAL);
  if (n == 0) {
    minitel.moveCursorXY(1, 7);
    minitel.print(F("Aucune page sur la carte SD."));
    minitel.moveCursorXY(1, 9);
    minitel.print(F("Copiez des fichiers .VDT (pages"));
    minitel.moveCursorXY(1, 10);
    minitel.print(F("Minitel) à la racine de la carte."));
    minitel.moveCursorXY(1, 24);
    afficherTouche(F("SOMMAIRE"));
    minitel.print(F(" : retour"));
    return;
  }
  minitel.moveCursorXY(1, 7);
  minitel.print(String(n));
  minitel.print(n == 1 ? F(" page sur la carte SD") : F(" pages sur la carte SD"));
  minitel.moveCursorXY(1, 10);
  afficherTouche(F("SUITE"));
  minitel.print(F(" : page suivante"));
  minitel.moveCursorXY(1, 12);
  afficherTouche(F("RETOUR"));
  minitel.print(F(" : page précédente"));
  minitel.moveCursorXY(1, 14);
  afficherTouche(F("ENVOI"));
  minitel.print(F(" : défilement automatique"));
  minitel.moveCursorXY(1, 16);
  afficherTouche(F("SOMMAIRE"));
  minitel.print(F(" : retour au menu"));
}

//////////////////////////////////////////////////////////////////////////////////////

unsigned long vdtAttendre(boolean defilement, unsigned long debut)
{
  // Attend SUITE, RETOUR, ENVOI ou SOMMAIRE. En défilement automatique, renvoie SUITE
  // quand la page est restée affichée VDT_DUREE millisecondes depuis debut.
  while (true) {
    unsigned long k = minitel.getKeyCode();
    if (k == SOMMAIRE || k == SUITE || k == RETOUR || k == ENVOI) return k;
    if (defilement && millis() - debut >= VDT_DUREE) return SUITE;
  }
}

//////////////////////////////////////////////////////////////////////////////////////

void pagesVideotex()
{
  int n = vdtNombreDePages();
  vdtEcranPresentation(n);
  if (n == 0) {
    while (minitel.getKeyCode() != SOMMAIRE);
    return;
  }

  // Sur l'écran de présentation : SUITE ou ENVOI (défilement automatique) affiche la
  // première page, SOMMAIRE revient au menu. Ensuite, on ne voit que les pages.
  unsigned long touche = 0;
  while (touche != SUITE && touche != ENVOI && touche != SOMMAIRE) touche = minitel.getKeyCode();
  if (touche == SOMMAIRE) return;
  boolean defilement = (touche == ENVOI);
  if (defilement) bipOK();

  char page[VDT_NOM_MAX];
  char voisine[VDT_NOM_MAX];
  page[0] = 0;
  int sens = 1;
  boolean changer = true;  // false : on reste sur la page affichée
  unsigned long debut = 0;
  while (true) {
    touche = 0;
    if (changer) {
      // Page suivante ou précédente ; au bout de la liste, on repart de l'autre bout
      if (!vdtVoisine(page, sens, voisine) && !vdtVoisine("", sens, voisine)) break;
      strcpy(page, voisine);
      touche = vdtAfficher(page);  // Une touche frappée pendant l'envoi compte tout de suite
      debut = millis();
    }
    if (touche == 0) touche = vdtAttendre(defilement, debut);
    if (touche == SOMMAIRE) break;
    if (touche == ENVOI) {
      // Défilement automatique, ou arrêt : on reste sur la page
      defilement = !defilement;
      if (defilement) bipOK(); else minitel.bip();
      debut = millis();
      changer = false;
    }
    else {
      sens = (touche == RETOUR) ? -1 : 1;
      changer = true;
    }
  }

  // Une page peut avoir changé les réglages de l'écran : on revient à l'affichage normal
  minitel.newScreen();
  vdtEffacerRangee0();
  minitel.textMode();
  minitel.noCursor();
}

//////////////////////////////////////////////////////////////////////////////////////
