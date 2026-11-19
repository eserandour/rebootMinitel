//////////////////////////////////////////////////////////////////////////////////////

// MENUS

//////////////////////////////////////////////////////////////////////////////////////

void selectMenu(byte tag)
{
  ecran = tag;
  minitel.newScreen();
}

//////////////////////////////////////////////////////////////////////////////////////

void ecranAccueil()
{
  boolean fin = false;
  boolean premierPassage = true;
  while (!fin) {
    touche = minitel.getKeyCode();
    switch (touche) {
      case 0x30 :  // Touche 0
        fin = true;
        selectMenu(TAG_MENU_REGLAGE_HORLOGE);
        break;
      case 0x31 :  // Touche 1
        fin = true;
        selectMenu(TAG_MENU_BASIC);
        break;
     case 0x32 :  // Touche 2
        fin = true;
        selectMenu(TAG_MENU_VIDEOTEX);
        break;
     case 0x33 :  // Touche 3
        fin = true;
        selectMenu(TAG_MENU_CARTE_SD);
        break;
     case 0x34 :  // Touche 4
        fin = true;
        selectMenu(TAG_MENU_PONG);
        break;
     case 0x35 :  // Touche 5
        fin = true;
        selectMenu(TAG_MENU_A_PROPOS);
        break;
     default:
       if (premierPassage) {
         premierPassage=false;
         forcerAffichageHorloge();  // L'écran est effacé : l'heure doit être réaffichée
         minitel.newScreen();
         minitel.echo(false);  // Les caractères ne s'affichent pas à l'écran
         minitel.noCursor();
         // entropie
         // Font: slscript
         // Source : http://www.network-science.de/ascii/
         minitel.moveCursorRight(15); minitel.println(F("_/_"));
         minitel.moveCursorRight(6); minitel.println(F("_  ____  /  __  __ _   o _"));
         minitel.moveCursorRight(5); minitel.println(F("</_/ / <_<__/ (_(_)/_)_<_</_"));
         minitel.moveCursorRight(23); minitel.println(F("/"));
         minitel.moveCursorRight(22); minitel.println(F("'"));
         // LAB
         // Font: graffiti
         // Source : http://www.network-science.de/ascii/
         // Attention : Les caractères \ doivent être doublés pour s'afficher
         minitel.moveCursorRight(5); minitel.println(F(".____       _____ __________"));
         minitel.moveCursorRight(5); minitel.println(F("|    |     /  _  \\\\______   \\"));
         minitel.moveCursorRight(5); minitel.println(F("|    |    /  /_\\  \\|    |  _/"));
         minitel.moveCursorRight(5); minitel.println(F("|    |___/    |    \\    |   \\"));
         minitel.moveCursorRight(5); minitel.println(F("|_______ \\____|__  /______  /"));
         minitel.moveCursorRight(13); minitel.println(F("\\/       \\/       \\/"));
         // Nom et version
         minitel.moveCursorDown(2);
         minitel.println(F("rebootMinitel"));
         minitel.println("V." + VERSION);

         // MENU
         minitel.moveCursorXY(20,14);
         //minitel.print("puis ");
         minitel.attributs(INVERSION_FOND);
         minitel.print(F("MENU"));
         minitel.attributs(FOND_NORMAL);
         minitel.moveCursorXY(20,16);
         minitel.println(F("0 - Réglage Horloge"));
         minitel.moveCursorXY(20,17);
         minitel.println(F("1 - reBASIC"));
         minitel.moveCursorXY(20,18);
         minitel.println(F("2 - Pages Vidéotex"));
         minitel.moveCursorXY(20,19);
         minitel.println(F("3 - Fichiers"));
         minitel.moveCursorXY(20,20);
         minitel.println(F("4 - Pong"));
         minitel.moveCursorXY(20,21);
         minitel.println(F("5 - À propos"));
       }
       afficherHorloge(11,24);
       break;
    }
  }
}

//////////////////////////////////////////////////////////////////////////////////////

void menuReglageHorloge()
{
  reglageHorloge();
  selectMenu(TAG_ECRAN_ACCUEIL);
}

//////////////////////////////////////////////////////////////////////////////////////

void menuBasic()
{
  basic();
  selectMenu(TAG_ECRAN_ACCUEIL);
}

//////////////////////////////////////////////////////////////////////////////////////

void menuVideotex()
{
  pagesVideotex();
  selectMenu(TAG_ECRAN_ACCUEIL);
}

//////////////////////////////////////////////////////////////////////////////////////

void menuCarteSD()
{
  carteSD();
  selectMenu(TAG_ECRAN_ACCUEIL);
}

//////////////////////////////////////////////////////////////////////////////////////

void menuPong()
{
  pong();
  selectMenu(TAG_ECRAN_ACCUEIL);
}

//////////////////////////////////////////////////////////////////////////////////////

void menuAPropos()
{
  aPropos();
  selectMenu(TAG_ECRAN_ACCUEIL);
}

//////////////////////////////////////////////////////////////////////////////////////

void afficherTouche(const __FlashStringHelper* touche)
{
  // Affiche le nom d'une touche du Minitel en vidéo inverse (comme dans l'aide du BASIC).
  // Le texte qui suit revient à l'affichage normal.
  minitel.attributs(INVERSION_FOND);
  minitel.print(touche);
  minitel.attributs(FOND_NORMAL);
}

//////////////////////////////////////////////////////////////////////////////////////
