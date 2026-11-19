//////////////////////////////////////////////////////////////////////////////////////

// HORLOGE

//////////////////////////////////////////////////////////////////////////////////////

/*
  Deux horloges possibles :
  * l'horloge RTC DS3231 du shield, si elle est là : elle garde l'heure, même éteinte ;
  * sinon (shield sans horloge, ESP32 seul), une horloge logicielle, qui compte avec
    millis() à partir de l'heure qu'on lui a donnée. Elle prend l'heure de l'ordinateur,
    par le câble USB, si la passerelle (outils/passerelle/passerelle.py) tourne : au
    démarrage, puis toutes les 5 secondes depuis l'écran d'accueil tant qu'elle n'a pas
    l'heure, et toutes les 10 minutes ensuite pour corriger sa dérive. On peut aussi la
    régler à la main (menu 0). Elle perd l'heure quand la carte s'éteint ou redémarre.

  Dialogue avec la passerelle : -> @HEURE    <- @HEURE JJ/MM/AAAA hh:mm:ss
*/

boolean horlogeOK = false;           // L'heure est-elle connue (horloge RTC, ou heure reçue) ?
boolean horlogeRTC = false;          // L'horloge RTC a-t-elle été détectée ?
RTC_Millis horlogeLogicielle;        // L'horloge logicielle, sans horloge RTC
int horlogeSeconde = -1;             // Seconde affichée (-1 : à réafficher)
unsigned long horlogeLecture = 0;    // Moment de la dernière lecture de l'horloge (millis)
unsigned long horlogeDemande = 0;    // Moment de la dernière demande de l'heure au PC (millis)
char horlogeTampon[28];              // Ligne en cours de réception depuis le PC
byte horlogeN = 0;                   // Nombre de caractères dans horlogeTampon

#define HORLOGE_PC_ATTENTE   5000UL    // Sans l'heure : nouvelle demande au PC toutes les 5 s
#define HORLOGE_PC_DERIVE    600000UL  // Avec l'heure du PC : remise à l'heure toutes les 10 min

//////////////////////////////////////////////////////////////////////////////////////

DateTime horlogeMaintenant()
{
  // La date et l'heure, quelle que soit l'horloge (à n'appeler que si horlogeOK)
  return horlogeRTC ? rtc.now() : horlogeLogicielle.now();
}

//////////////////////////////////////////////////////////////////////////////////////

void horlogeRegler(const DateTime& t)
{
  // Met à l'heure l'horloge en service
  if (horlogeRTC) rtc.adjust(t);
  else horlogeLogicielle.adjust(t);
  horlogeOK = true;
  horlogeSeconde = -1;  // A réafficher
}

//////////////////////////////////////////////////////////////////////////////////////

void horlogeDemanderPC()
{
  Serial.print(F("@HEURE\n"));
  horlogeDemande = millis();
}

//////////////////////////////////////////////////////////////////////////////////////

boolean horlogeLirePC()
{
  // Lit, sans attendre, ce qui est arrivé du PC. Vrai si l'heure a été reçue (et réglée).
  // Les autres lignes sont ignorées.
  boolean recue = false;
  while (Serial.available()) {
    int c = Serial.read();
    if (c < 0 || c == '\r') continue;
    if (c == '\n') {
      horlogeTampon[horlogeN] = 0;
      horlogeN = 0;
      if (horlogeDecoderPC(horlogeTampon)) recue = true;
    }
    else if (horlogeN < sizeof(horlogeTampon) - 1) {
      horlogeTampon[horlogeN++] = (char) c;
    }
  }
  return recue;
}

//////////////////////////////////////////////////////////////////////////////////////

boolean horlogeDecoderPC(const char* t)
{
  // "@HEURE JJ/MM/AAAA hh:mm:ss" : règle l'horloge si la date et l'heure sont valides
  if (strncmp_P(t, PSTR("@HEURE "), 7) != 0 || strlen(t) != 26) return false;
  t += 7;
  // Les chiffres, aux positions de "JJ/MM/AAAA hh:mm:ss"
  const byte position[14] = { 0, 1, 3, 4, 6, 7, 8, 9, 11, 12, 14, 15, 17, 18 };
  for (byte i = 0; i < 14; i++) {
    if (t[position[i]] < '0' || t[position[i]] > '9') return false;
  }
  int jours = deuxChiffres(t);
  int mois = deuxChiffres(t + 3);
  int annees = deuxChiffres(t + 6) * 100 + deuxChiffres(t + 8);
  int heures = deuxChiffres(t + 11);
  int minutes = deuxChiffres(t + 14);
  int secondes = deuxChiffres(t + 17);
  if (!dateValide(jours, mois, annees) || heures > 23 || minutes > 59 || secondes > 59) return false;
  horlogeRegler(DateTime(annees, mois, jours, heures, minutes, secondes));
  return true;
}

//////////////////////////////////////////////////////////////////////////////////////

boolean horlogeAttendrePC(unsigned long duree)
{
  // Demande l'heure au PC et attend la réponse, au plus duree millisecondes
  while (Serial.available()) Serial.read();  // On oublie ce qui a pu arriver avant
  horlogeN = 0;
  horlogeDemanderPC();
  unsigned long debut = millis();
  while (millis() - debut < duree) {
    if (horlogeLirePC()) return true;
  }
  return false;
}

//////////////////////////////////////////////////////////////////////////////////////

void horlogeSurveillerPC()
{
  // Sans horloge RTC : demande régulièrement l'heure au PC, sans jamais attendre
  // (appelée par afficherHorloge, depuis l'écran d'accueil)
  if (horlogeRTC) return;
  horlogeLirePC();
  unsigned long delai = horlogeOK ? HORLOGE_PC_DERIVE : HORLOGE_PC_ATTENTE;
  if (millis() - horlogeDemande >= delai) horlogeDemanderPC();
}

//////////////////////////////////////////////////////////////////////////////////////

void initHorloge()
{
  horlogeRTC = rtc.begin();
  horlogeOK = horlogeRTC;
  if (horlogeRTC) {
    if (rtc.lostPower()) {
      // L'heure du PC, si la passerelle répond ; sinon la date et l'heure de compilation
      if (horlogeAttendrePC(1500)) {
        Serial.println(F("L'horloge avait perdu l'heure : mise à l'heure du PC"));
      }
      else {
        Serial.println(F("L'horloge a perdu l'heure : mise à l'heure de la compilation"));
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
      }
      // Pour une date et une heure précises, par exemple le 21 janvier 2014 à 3 h :
      // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    }
    if (!testHorloge()) {
      rtc.adjust(DateTime(2000,1,1,0,0,0));
      // Si l'heure lue reste incohérente, ce n'est pas une horloge qui répond : rtc.begin()
      // peut réussir sans horloge (bus I2C sans résistances de tirage, autre composant à
      // l'adresse 0x68...). Les octets lus valent alors 0xFF, ce qui donne 165/165/2165.
      horlogeRTC = testHorloge();
      horlogeOK = horlogeRTC;
    }
  }
  if (!horlogeRTC) {
    // Pas d'horloge RTC : l'horloge logicielle prend l'heure du PC, s'il répond
    horlogeLogicielle.begin(DateTime(2000, 1, 1, 0, 0, 0));
    if (horlogeAttendrePC(1500)) return;
    // Ni horloge ni PC : on le signale sur le Minitel et le programme continue sans
    // l'heure (l'écran d'accueil continue à la demander au PC, voir horlogeSurveillerPC)
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
  // L'heure lue dans l'horloge RTC est-elle cohérente ?
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
  horlogeSurveillerPC();
  if (!horlogeOK) return;
  if (horlogeSeconde >= 0 && millis() - horlogeLecture < 100) return;
  horlogeLecture = millis();
  DateHeure = horlogeMaintenant();
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
  // Sans horloge RTC, c'est l'horloge logicielle qui est réglée : elle garde l'heure
  // jusqu'au prochain redémarrage de la carte.

  // Affichage
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
  minitel.print(F(" : retour au menu sans modifier"));

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
    horlogeRegler(DateTime(annees, mois, jours, heures, minutes, 0));
    bipOK();
  }
  else {
    bipErreur();
  }
}

//////////////////////////////////////////////////////////////////////////////////////
