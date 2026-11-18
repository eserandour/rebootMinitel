//////////////////////////////////////////////////////////////////////////////////////

// HORLOGE

//////////////////////////////////////////////////////////////////////////////////////

boolean horlogeOK = false;           // L'horloge RTC a-t-elle été détectée ?
int horlogeSeconde = -1;             // Seconde affichée (-1 : à réafficher)
unsigned long horlogeLecture = 0;    // Moment de la dernière lecture de l'horloge (millis)

//////////////////////////////////////////////////////////////////////////////////////

void initHorloge()
{
  horlogeOK = rtc.begin();
  if (horlogeOK) {
    if (rtc.lostPower()) {
      Serial.println(F("L'horloge a perdu l'heure : mise à l'heure de la compilation"));
      // Règle l'horloge sur la date et l'heure de compilation du croquis
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
      // Pour une date et une heure précises, par exemple le 21 janvier 2014 à 3 h :
      // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    }
    if (!testHorloge()) {
      rtc.adjust(DateTime(2000,1,1,0,0,0));
      // Si l'heure lue reste incohérente, ce n'est pas une horloge qui répond : rtc.begin()
      // peut réussir sans horloge (bus I2C sans résistances de tirage, autre composant à
      // l'adresse 0x68...). Les octets lus valent alors 0xFF, ce qui donne 165/165/2165.
      horlogeOK = testHorloge();
    }
  }
  if (!horlogeOK) {
    // Pas d'horloge : on le signale sur le Minitel et le programme continue sans elle
    Serial.println(F("Horloge introuvable"));
    minitel.moveCursorXY(11, 23);
    minitel.print(F("Horloge introuvable"));
    bipErreur();
    delay(3000);
  }
}

//////////////////////////////////////////////////////////////////////////////////////

boolean testHorloge()
{
  DateHeure = rtc.now();
  int annees = DateHeure.year();
  int mois = DateHeure.month();
  int jours = DateHeure.day();
  int heures = DateHeure.hour();
  int minutes = DateHeure.minute();
  int secondes = DateHeure.second();
  return dateValide(jours, mois, annees) &&
         heures >= 0 && heures <= 23 &&
         minutes >= 0 && minutes <= 59 &&
         secondes >= 0 && secondes <= 59;
}

//////////////////////////////////////////////////////////////////////////////////////

boolean dateValide(int jours, int mois, int annees)
{
  // Vérifie que la date existe (31/04 ou 29/02/2027 sont refusés)
  if (annees < 2000 || annees > 2099 || mois < 1 || mois > 12 || jours < 1) return false;
  int joursDansLeMois = 31;
  if (mois == 4 || mois == 6 || mois == 9 || mois == 11) {
    joursDansLeMois = 30;
  }
  else if (mois == 2) {
    joursDansLeMois = (annees % 4 == 0) ? 29 : 28;  // Exact de 2000 à 2099
  }
  return jours <= joursDansLeMois;
}

//////////////////////////////////////////////////////////////////////////////////////

void forcerAffichageHorloge()
{
  // A appeler après avoir effacé l'écran : l'heure sera réaffichée au prochain appel
  horlogeSeconde = -1;
}

//////////////////////////////////////////////////////////////////////////////////////

void afficherHorloge(int x, int y)
{
  // Peut être appelée aussi souvent que l'on veut : l'horloge n'est lue que tous les
  // dixièmes de seconde, et l'écran n'est mis à jour que lorsque la seconde change.
  if (!horlogeOK) return;
  if (horlogeSeconde >= 0 && millis() - horlogeLecture < 100) return;
  horlogeLecture = millis();
  DateHeure = rtc.now();
  if (DateHeure.second() == horlogeSeconde) return;
  horlogeSeconde = DateHeure.second();
  formaterDateHeure();
  char texte[21];  // "JJ/MM/AAAA  hh:mm:ss"
  snprintf_P(texte, sizeof(texte), PSTR("%s  %s"), Date, Heure);
  minitel.noCursor();
  minitel.moveCursorXY(x,y);
  minitel.print(texte);
}

//////////////////////////////////////////////////////////////////////////////////////

void formaterDateHeure()
{
  // Remplit Date ("JJ/MM/AAAA") et Heure ("hh:mm:ss") à partir de DateHeure
  snprintf_P(Date, sizeof(Date), PSTR("%02d/%02d/%04d"),
           DateHeure.day(), DateHeure.month(), DateHeure.year());
  snprintf_P(Heure, sizeof(Heure), PSTR("%02d:%02d:%02d"),
           DateHeure.hour(), DateHeure.minute(), DateHeure.second());
}

//////////////////////////////////////////////////////////////////////////////////////

void reglageHorloge()
{
  if (!horlogeOK) {
    minitel.moveCursorXY(11,12);
    minitel.print(F("Horloge introuvable"));
    minitel.moveCursorXY(12,24);  // "SOMMAIRE : retour" centré (17 caractères)
    afficherTouche(F("SOMMAIRE"));
    minitel.print(F(" : retour"));
    bipErreur();
    while (minitel.getKeyCode() != SOMMAIRE);
    return;
  }

  // Affichage
  DateHeure = rtc.now();
  formaterDateHeure();
  const int colonneDebut = 12;  // Pour centrer "JJ/MM/AAAA  hh:mm" (17 caractères)
  minitel.moveCursorXY(colonneDebut,11);
  minitel.print(F("JJ/MM/AAAA  hh:mm"));
  minitel.moveCursorXY(colonneDebut,12);
  minitel.print(F("  /  /        :  "));
  minitel.moveCursorXY(1,23);
  afficherTouche(F("CORRECTION"));
  minitel.print(F(" : effacer"));
  minitel.moveCursorXY(1,24);
  afficherTouche(F("SOMMAIRE"));
  minitel.print(F(" : retour sans modifier"));

  // Saisie des 12 chiffres. Position de chaque chiffre dans "JJ/MM/AAAA  hh:mm" :
  const byte position[12] = { 0, 1, 3, 4, 6, 7, 8, 9, 12, 13, 15, 16 };
  char chaine[13];
  int n = 0;
  boolean abandon = false;
  minitel.moveCursorXY(colonneDebut + position[0],12);
  minitel.cursor();
  while (n < 12 && !abandon) {
    touche = minitel.getKeyCode();
    if (touche >= 0x30 && touche <= 0x39) {  // Touches 0 à 9
      minitel.printChar(touche);
      chaine[n++] = (char) touche;
      if (n < 12) {
        minitel.moveCursorXY(colonneDebut + position[n],12);  // On saute les / , les espaces et le :
      }
    }
    else if (touche == CORRECTION && n > 0) {
      n--;
      minitel.moveCursorXY(colonneDebut + position[n],12);
      minitel.printChar(' ');
      minitel.moveCursorXY(colonneDebut + position[n],12);
    }
    else if (touche == SOMMAIRE) {
      abandon = true;  // On quitte sans rien changer, et sans bip d'erreur
    }
  }
  chaine[n] = 0;
  minitel.noCursor();
  if (!abandon) {
    decodeChaineHorloge(chaine);  // Pour la mise à l'heure de l'horloge
  }
}

//////////////////////////////////////////////////////////////////////////////////////

int deuxChiffres(const char* p)
{
  return (p[0] - '0') * 10 + (p[1] - '0');
}

//////////////////////////////////////////////////////////////////////////////////////

void decodeChaineHorloge(const char* chaine)
{
  // chaine : 12 chiffres JJMMAAAAhhmm
  int jours = deuxChiffres(chaine);
  int mois = deuxChiffres(chaine + 2);
  int annees = deuxChiffres(chaine + 4) * 100 + deuxChiffres(chaine + 6);
  int heures = deuxChiffres(chaine + 8);
  int minutes = deuxChiffres(chaine + 10);
  if (dateValide(jours, mois, annees) &&
      heures >= 0 && heures <= 23 &&
      minutes >= 0 && minutes <= 59) {
    rtc.adjust(DateTime(annees, mois, jours, heures, minutes, 0));
    bipOK();
  }
  else {
    bipErreur();
  }
}

//////////////////////////////////////////////////////////////////////////////////////
