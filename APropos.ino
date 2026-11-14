//////////////////////////////////////////////////////////////////////////////////////

// À PROPOS (LOGICIEL / AUTEUR)

//////////////////////////////////////////////////////////////////////////////////////

void aPropos()
{
  String texte;
  minitel.noCursor();
  
  texte = F("rebootMinitel (2023-2026)");
  int x = (40 - texte.length()) / 2 + 1; 
  minitel.moveCursorXY(x, 9);
  minitel.print(texte);
  
  texte = F("Auteur : Eric Sérandour");
  minitel.moveCursorXY(x, 11);
  minitel.print(texte);

  texte = F("Logiciel Libre (GNU GPLv3)");
  minitel.moveCursorXY(x, 13);
  minitel.print(texte);

  texte = F("https://entropie.org/3615");
  minitel.moveCursorXY(x, 15);
  minitel.print(texte);

  minitel.moveCursorXY(1, 24);
  afficherTouche(F("SOMMAIRE"));
  minitel.print(F(" : retour"));

  while (minitel.getKeyCode() != SOMMAIRE);
}
