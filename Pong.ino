//////////////////////////////////////////////////////////////////////////////////////

// PONG (menu 4)

//////////////////////////////////////////////////////////////////////////////////////
/*
  Le jeu de Pong en semi-graphique (80 x 72 points), à un joueur contre le Minitel, à
  deux joueurs sur le même clavier, ou Minitel contre Minitel (démonstration sans fin).

    Joueur de gauche : A monte, Q descend
    Joueur de droite : P monte, M descend (à deux joueurs)
    SOMMAIRE         : retour au menu

  Le premier à PONG_GAGNANT points gagne (sauf en démonstration, qui continue jusqu'à
  SOMMAIRE). Le dessin passe par la mémoire de points de
  reBASIC (basGfx, 720 octets) : le jeu n'utilise presque pas de mémoire en plus. On ne
  renvoie au Minitel que les cases qui changent, car il n'affiche que 480 caractères par
  seconde à 4800 bauds.
*/

const byte PONG_HAUT = 3;          // Le mur du haut (en points), rangée 2 de l'écran
const byte PONG_BAS = 68;          // Le mur du bas, rangée 23 (la rangée 24 reste au texte)
const byte PONG_X_GAUCHE = 3;      // Colonne de la raquette de gauche
const byte PONG_X_DROITE = 76;     // Colonne de la raquette de droite
const byte PONG_RAQUETTE = 9;      // Hauteur d'une raquette, en points
const byte PONG_PAS = 3;           // Déplacement d'une raquette à chaque touche
const byte PONG_GAGNANT = 5;       // Points pour gagner
const unsigned long PONG_IMAGE = 60;  // Durée d'une image, en millisecondes

// Positions en 1/16 de point, pour des trajectoires régulières
int pongBx, pongBy, pongVx, pongVy;
int pongG, pongD;                  // Haut des raquettes (en points)
int pongScoreG, pongScoreD;

//////////////////////////////////////////////////////////////////////////////////////

boolean pongFixe(int x, int y)
{
  // Ce point appartient-il au filet ou à une raquette ? (la balle ne doit pas l'effacer)
  if (x == 39 && y > PONG_HAUT && y < PONG_BAS && (y - PONG_HAUT - 2) % 4 == 0) return true;
  if (x == PONG_X_GAUCHE && y >= pongG && y < pongG + PONG_RAQUETTE) return true;
  if (x == PONG_X_DROITE && y >= pongD && y < pongD + PONG_RAQUETTE) return true;
  return false;
}

//////////////////////////////////////////////////////////////////////////////////////

void pongBalle(byte* cases, boolean allume)
{
  // La balle : 2 x 2 points
  int x = pongBx >> 4, y = pongBy >> 4;
  for (int dy = 0; dy < 2; dy++) {
    for (int dx = 0; dx < 2; dx++) {
      if (allume || !pongFixe(x + dx, y + dy)) basGfxMark(cases, x + dx, y + dy, allume);
    }
  }
}

//////////////////////////////////////////////////////////////////////////////////////

void pongRaquette(byte* cases, int x, int haut, boolean allume)
{
  for (int y = haut; y < haut + PONG_RAQUETTE; y++) basGfxMark(cases, x, y, allume);
}

//////////////////////////////////////////////////////////////////////////////////////

void pongScore()
{
  // Le score en haut, en double largeur (deux colonnes par chiffre), symétrique par rapport
  // au milieu de l'écran : le chiffre des unités de gauche (colonnes 17-18) fait face au
  // premier chiffre de droite (colonnes 23-24). Les scores ne font que grandir : à gauche,
  // un nouveau chiffre s'ajoute devant ; à droite, derrière.
  char t[8];
  snprintf_P(t, sizeof(t), PSTR("%d"), pongScoreG);
  minitel.moveCursorXY(19 - 2 * strlen(t), 1);
  minitel.attributs(DOUBLE_LARGEUR);
  minitel.print(t);
  snprintf_P(t, sizeof(t), PSTR("%d"), pongScoreD);
  minitel.moveCursorXY(23, 1);
  minitel.attributs(DOUBLE_LARGEUR);  // Un positionnement remet la taille normale
  minitel.print(t);
  minitel.attributs(GRANDEUR_NORMALE);
}

//////////////////////////////////////////////////////////////////////////////////////

void pongService(int vers)
{
  // La balle repart du milieu, vers la gauche (vers < 0) ou vers la droite
  pongBx = 39 << 4;
  pongBy = ((PONG_HAUT + PONG_BAS) / 2) << 4;
  pongVx = (vers < 0) ? -16 : 16;
  pongVy = random(-12, 13);
}

//////////////////////////////////////////////////////////////////////////////////////

int pongBouger(int haut, int dy)
{
  // Une raquette monte (dy < 0) ou descend, sans sortir du terrain
  haut += dy;
  if (haut < PONG_HAUT + 1) haut = PONG_HAUT + 1;
  if (haut > PONG_BAS - PONG_RAQUETTE) haut = PONG_BAS - PONG_RAQUETTE;
  return haut;
}

//////////////////////////////////////////////////////////////////////////////////////

int pongAutomate(int haut, int x)
{
  // Une raquette jouée par le Minitel : elle suit la balle quand celle-ci vient vers elle,
  // d'un point par image au plus (elle finit donc par rater une balle rapide)
  boolean vient = (x == PONG_X_DROITE) ? (pongVx > 0 && pongBx > (30 << 4)) : (pongVx < 0 && pongBx < (49 << 4));
  if (!vient) return haut;
  int cible = (pongBy >> 4) - PONG_RAQUETTE / 2;
  if (cible >= haut - 1 && cible <= haut + 1) return haut;
  return pongBouger(haut, (cible < haut) ? -1 : 1);
}

//////////////////////////////////////////////////////////////////////////////////////

byte pongPartie(byte mode)
{
  // Une partie. mode : 0 Minitel contre Minitel (sans fin), 1 un joueur, 2 deux joueurs.
  // Renvoie 0 si l'on a quitté (SOMMAIRE), 1 ou 2 pour le gagnant (gauche, droite).
  boolean deuxJoueurs = (mode == 2);
  byte cases[120];
  minitel.newScreen();
  minitel.noCursor();
  basGfxClear();
  memset(cases, 0, sizeof(cases));
  for (int x = 0; x < 80; x++) {          // Les murs
    basGfxMark(cases, x, PONG_HAUT, true);
    basGfxMark(cases, x, PONG_BAS, true);
  }
  for (int y = PONG_HAUT + 2; y < PONG_BAS; y += 4) basGfxMark(cases, 39, y, true);  // Le filet
  pongG = pongD = (PONG_HAUT + PONG_BAS - PONG_RAQUETTE) / 2;
  pongRaquette(cases, PONG_X_GAUCHE, pongG, true);
  pongRaquette(cases, PONG_X_DROITE, pongD, true);
  basGfxFlush(cases);
  pongScoreG = pongScoreD = 0;
  pongScore();

  pongService(random(0, 2) ? 1 : -1);
  unsigned long image = millis();
  unsigned long attente = millis() + 1000;  // Une seconde avant le premier service
  while (true) {
    // Le clavier : on le lit sans arrêt, une touche déplace aussitôt sa raquette
    unsigned long k = minitel.getKeyCode();
    if (k == SOMMAIRE) return 0;
    if (k >= 'a' && k <= 'z') k -= 32;
    if (((k == 'A' || k == 'Q') && mode != 0) || ((k == 'P' || k == 'M') && deuxJoueurs)) {
      memset(cases, 0, sizeof(cases));
      boolean gauche = (k == 'A' || k == 'Q');
      int* haut = gauche ? &pongG : &pongD;
      int x = gauche ? PONG_X_GAUCHE : PONG_X_DROITE;
      pongRaquette(cases, x, *haut, false);
      *haut = pongBouger(*haut, (k == 'A' || k == 'P') ? -PONG_PAS : PONG_PAS);
      pongRaquette(cases, x, *haut, true);
      basGfxFlush(cases);
    }
    if (millis() - image < PONG_IMAGE) continue;
    image = millis();
    if ((long) (millis() - attente) < 0) continue;  // Pause avant un service

    memset(cases, 0, sizeof(cases));
    // Le Minitel joue à droite (sauf à deux joueurs), et aussi à gauche en démonstration
    if (!deuxJoueurs) {
      int haut = pongAutomate(pongD, PONG_X_DROITE);
      if (haut != pongD) {
        pongRaquette(cases, PONG_X_DROITE, pongD, false);
        pongD = haut;
        pongRaquette(cases, PONG_X_DROITE, pongD, true);
      }
    }
    if (mode == 0) {
      int haut = pongAutomate(pongG, PONG_X_GAUCHE);
      if (haut != pongG) {
        pongRaquette(cases, PONG_X_GAUCHE, pongG, false);
        pongG = haut;
        pongRaquette(cases, PONG_X_GAUCHE, pongG, true);
      }
    }

    // La balle avance
    pongBalle(cases, false);
    pongBx += pongVx;
    pongBy += pongVy;
    int x = pongBx >> 4, y = pongBy >> 4;
    if (y <= PONG_HAUT) { pongBy = (PONG_HAUT + 1) << 4; pongVy = -pongVy; tone(BUZZER, 440, 20); }
    if (y + 1 >= PONG_BAS) { pongBy = (PONG_BAS - 2) << 4; pongVy = -pongVy; tone(BUZZER, 440, 20); }
    y = pongBy >> 4;
    // Les raquettes : l'angle de renvoi dépend de l'endroit où la balle les touche
    if (pongVx < 0 && x <= PONG_X_GAUCHE + 1 && x >= PONG_X_GAUCHE - 1 &&
        y + 1 >= pongG && y <= pongG + PONG_RAQUETTE - 1) {
      pongBx = (PONG_X_GAUCHE + 1) << 4;
      pongVx = -pongVx + 1;  // La balle accélère un peu à chaque renvoi
      pongVy = (y - (pongG + PONG_RAQUETTE / 2)) * 5;
      tone(BUZZER, 880, 20);
    }
    if (pongVx > 0 && x + 1 >= PONG_X_DROITE - 1 && x + 1 <= PONG_X_DROITE + 1 &&
        y + 1 >= pongD && y <= pongD + PONG_RAQUETTE - 1) {
      pongBx = (PONG_X_DROITE - 2) << 4;
      pongVx = -pongVx - 1;
      pongVy = (y - (pongD + PONG_RAQUETTE / 2)) * 5;
      tone(BUZZER, 880, 20);
    }
    if (pongVx > 40) pongVx = 40;     // Pas plus de 2,5 points par image
    if (pongVx < -40) pongVx = -40;

    // Un point est marqué quand la balle sort du terrain
    x = pongBx >> 4;
    if (x < 0 || x > 78) {
      if (x < 0) pongScoreD++; else pongScoreG++;
      tone(BUZZER, 220, 150);
      basGfxFlush(cases);  // On efface la balle
      pongScore();
      if (mode != 0 && pongScoreG == PONG_GAGNANT) return 1;
      if (mode != 0 && pongScoreD == PONG_GAGNANT) return 2;
      pongService(x < 0 ? 1 : -1);  // Le service va vers celui qui vient de marquer
      attente = millis() + 1000;
      continue;
    }
    pongBalle(cases, true);
    basGfxFlush(cases);
  }
}

//////////////////////////////////////////////////////////////////////////////////////

void pong()
{
  randomSeed(millis());
  minitel.pageMode();  // reBASIC a pu laisser le mode rouleau
  while (true) {
    // Le choix du nombre de joueurs
    minitel.newScreen();
    minitel.noCursor();
    minitel.moveCursorXY(1, 3);
    minitel.attributs(INVERSION_FOND);
    minitel.print(F(" Pong "));
    minitel.attributs(FOND_NORMAL);
    minitel.moveCursorXY(1, 6);
    minitel.print(F("0 - Minitel contre Minitel"));
    minitel.moveCursorXY(1, 8);
    minitel.print(F("1 - Un joueur (contre le Minitel)"));
    minitel.moveCursorXY(1, 10);
    minitel.print(F("2 - Deux joueurs"));
    minitel.moveCursorXY(1, 14);
    minitel.print(F("Joueur de gauche : A monte, Q descend"));
    minitel.moveCursorXY(1, 16);
    minitel.print(F("Joueur de droite : P monte, M descend"));
    minitel.moveCursorXY(1, 19);
    minitel.print(F("Le premier à 5 points gagne"));
    minitel.moveCursorXY(1, 20);
    minitel.print(F("(Minitel contre Minitel : sans fin)."));
    minitel.moveCursorXY(1, 24);
    afficherTouche(F("SOMMAIRE"));
    minitel.print(F(" : retour au menu"));
    unsigned long k = 0;
    while (k != '0' && k != '1' && k != '2' && k != SOMMAIRE) k = minitel.getKeyCode();
    if (k == SOMMAIRE) break;
    byte mode = k - '0';

    byte gagnant = pongPartie(mode);
    if (gagnant == 0) break;
    // Fin de la partie : le gagnant et ENVOI, centrés sur la rangée du bas
    const __FlashStringHelper* message;
    if (mode == 1) message = (gagnant == 1) ? F("Vous avez gagné !") : F("Le Minitel a gagné !");
    else message = (gagnant == 1) ? F("La gauche a gagné !") : F("La droite a gagné !");
    // Pour un centrage exact, la ligne doit avoir un nombre pair de caractères : selon la
    // longueur du message, on met deux ou trois espaces avant ENVOI
    String texte = String(message);
    byte longueur = texte.length() - 1;  // Le é compte 2 octets mais 1 caractère
    byte espaces = (longueur % 2 == 1) ? 2 : 3;
    minitel.moveCursorXY((40 - (longueur + espaces + 15)) / 2 + 1, 24);
    minitel.print(texte);
    minitel.print(espaces == 2 ? F("  ") : F("   "));
    afficherTouche(F("ENVOI"));
    minitel.print(F(" : rejouer"));
    bipOK();
    k = 0;
    while (k != ENVOI && k != SOMMAIRE) k = minitel.getKeyCode();
    if (k == SOMMAIRE) break;
  }
  basGfxClear();
  minitel.newScreen();
}

//////////////////////////////////////////////////////////////////////////////////////
