//////////////////////////////////////////////////////////////////////////////////////

// À PROPOS (LOGICIEL / AUTEUR)

//////////////////////////////////////////////////////////////////////////////////////

void aProposLigne(const __FlashStringHelper* texte, byte x, byte rangee)
{
  minitel.moveCursorXY(x, rangee);
  minitel.print(texte);
}

//////////////////////////////////////////////////////////////////////////////////////

void aPropos()
{
  minitel.noCursor();

  // La première ligne est centrée ; les suivantes s'alignent sur elle. Le texte reste en
  // mémoire programme (F) : pas de variable String.
  static const char titre[] PROGMEM = "rebootMinitel (2023-2026)";
  byte x = (40 - strlen_P(titre)) / 2 + 1;
  aProposLigne((const __FlashStringHelper*) titre, x, 7);
  aProposLigne(F("Un projet d'Eric Sérandour"), x, 9);
  aProposLigne(F("basé sur sa bibliothèque"), x, 11);
  aProposLigne(F("Minitel1B_Hard (2016-2026)"), x, 13);
  aProposLigne(F("Logiciel Libre (GNU GPLv3)"), x, 15);
  aProposLigne(F("https://entropie.org/3615"), x, 17);

  minitel.moveCursorXY(1, 24);
  afficherTouche(F("SOMMAIRE"));
  minitel.print(F(" : retour au menu"));

  while (minitel.getKeyCode() != SOMMAIRE);
}
