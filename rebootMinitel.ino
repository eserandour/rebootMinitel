//////////////////////////////////////////////////////////////////////////////////////
/*
   rebootMinitel
   Copyright 2023-2026 - Eric Sérandour
*/
   const String VERSION = "2026.11.19d";
/*
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <https://www.gnu.org/licenses/>.
*/
//////////////////////////////////////////////////////////////////////////////////////
/*
  Cartes :
  * Arduino Mega 2560 + Shield 3615 Mega (version du 27/06/2023).
    Compilation réussie avec l'IDE Arduino 1.8.19.
  * ESP32 (carte esp32 d'Espressif Systems, version 3.x).

  Bibliothèques : Minitel1B_Hard, RTClib (Adafruit), SD (voir le README).
*/
//////////////////////////////////////////////////////////////////////////////////////
/*
  L'électronique du shield (liste non exhaustive) :

  * Module MicroSD Card Adapter de Catalex
  * Module horloge temps réel (RTC) DS3231

  Le circuit, sur l'Arduino Mega :

    Les entrées / sorties numériques :
  ======================================
  * Buzzer sur D2
    D2 -> R = 470 ohms -> Buzzer -> GND.

  * La carte SD est reliée au bus SPI (bus série synchrone) :
  ** SS   - D53 (voir la constante CHIP_SELECT)
  ** MOSI - D51 (Master Out Slave In : données envoyées à la carte)
  ** MISO - D50 (Master In Slave Out : données reçues de la carte)
  ** SCK  - D52 (Serial ClocK : synchronise les échanges)

    Les ports de communication :
  ======================================
  * L'horloge RTC est reliée à :
  ** Bus I2C : SDA (D20)
  ** Bus I2C : SCL (D21)
  ** SQW - D34
  ** 32K - D36

  * Les ports série UART :
  ** RX0 / TX0   - D0 / D1   : communication avec le PC (USB) : SAVE "PC:NOM"... de reBASIC
  ** RX0' / TX0'             : communication avec une carte en 3,3 V
  ** TX2 / RX2   - D16 / D17 : communication avec l'embase USB-A
  ** TX1 / RX1   - D18 / D19 : communication avec le Minitel

    Bilan :
  ===========
    Sont utilisées : D0, D1, D16, D17, D18, D19,   (ports série)
                     D2,                           (buzzer)
                     D20, D21,                     (I2C : SDA / SCL)
                     D34, D36,                     (SQW et 32K de l'horloge RTC)
                     D50, D51, D52, D53            (SPI de la carte SD)

  Sur l'ESP32 :
  * Minitel : Serial2, RX = GPIO16, TX = GPIO17
  * Horloge RTC : I2C, SDA = GPIO21, SCL = GPIO22
  * Carte SD : SPI (VSPI), CS = GPIO5, SCK = GPIO18, MISO = GPIO19, MOSI = GPIO23
  * Buzzer : GPIO2
  L'horloge et la carte SD sont facultatives : sans elles, rebootMinitel le signale
  au démarrage et continue. Sans horloge, il prend l'heure du PC (passerelle, voir
  Horloge.ino) ou on la règle à la main (menu 0).
*/

// #include <SPI.h> // Nécessaire avec Arduino 1.6.12, inutile avec Arduino 1.8.19

// *** Minitel
#include <Minitel1B_Hard.h>  // Voir https://github.com/eserandour/Minitel1B_Hard

#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)  // Pour ESP32
// Le troisième port série matériel de l'ESP32 (Serial2 / U2RXD U2TXD)
// est utilisé pour la connexion avec le Minitel : RX2 = GPIO16, TX2 = GPIO17.
// Les broches doivent être précisées : depuis la version 3.0 de la carte esp32
// d'Espressif Systems, Serial2 utilise par défaut GPIO4 (RX) et GPIO25 (TX).
// Sans elles, le Minitel ne répond jamais et searchSpeed() attend sans fin.
Minitel minitel(Serial2, 16, 17);  // Port série, broche RX, broche TX
#else  // Pour ATmega 1284P ou ATmega 2560 notamment
// Le deuxième port série matériel de l'ATMega (Serial1 / RXD1 TXD1)
// est utilisé pour la connexion avec le Minitel.
Minitel minitel(Serial1);
#endif

// *** Horloge RTC
#include <Wire.h>    // On importe la bibliothèque Wire pour l'I2C
#include <RTClib.h>  // Voir https://github.com/adafruit/RTClib
RTC_DS3231 rtc;

DateTime DateHeure;
char Date[11];   // "JJ/MM/AAAA"
char Heure[9];   // "hh:mm:ss"

// *** Carte SD
#include <SD.h>
#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
const byte CHIP_SELECT = 5;    // Chip Select de la carte SD pour l'ESP32 (SS du bus VSPI ; à adapter au montage)
#else
const byte CHIP_SELECT = 53;   // Chip Select de la carte SD pour l'Arduino Mega
#endif
File dataFile;

// *** Buzzer
const byte BUZZER = 2;

// *** Clavier
unsigned long touche;

// *** Ecrans
const byte TAG_ECRAN_ACCUEIL = 0;
const byte TAG_MENU_REGLAGE_HORLOGE = 1;
const byte TAG_MENU_BASIC = 2;
const byte TAG_MENU_A_PROPOS = 3;
const byte TAG_MENU_VIDEOTEX = 4;
const byte TAG_MENU_CARTE_SD = 5;
const byte TAG_MENU_PONG = 6;
byte ecran = TAG_ECRAN_ACCUEIL;



//////////////////////////////////////////////////////////////////////////////////////

void setup()
{
  // Liaison avec l'ordinateur, à 9600 bauds (câble USB) : SAVE "PC:NOM", LOAD "PC:NOM"...
  // de reBASIC, avec outils/passerelle/passerelle.py sur l'ordinateur
  Serial.begin(9600);

  // Minitel : on cherche sa vitesse, puis on passe à 4800 bauds
  minitel.changeSpeed(minitel.searchSpeed());
  minitel.changeSpeed(4800);
  minitel.newScreen();
  minitel.standardKeyboard();
  minitel.smallMode();

  // Liaison avec l'embase USB-A du shield, à 9600 bauds (non utilisée)
  // Serial2.begin(9600);

  // Buzzer
  pinMode(BUZZER, OUTPUT);
  
  // Bus I2C
  Wire.begin();

  // Horloge RTC DS3231 (I2C)
  initHorloge();

  // Carte SD
  // La broche Chip Select doit être en sortie
  pinMode(CHIP_SELECT, OUTPUT);
  // La carte est-elle présente et utilisable ?
  if (!SD.begin(CHIP_SELECT)) {
    minitel.moveCursorXY(11,24);
    minitel.print(F("Problème de carte SD"));
    bipErreur();  
    delay(3000);
  } else {
    bipOK();
  }
}

//////////////////////////////////////////////////////////////////////////////////////

void loop()
{ 
  switch(ecran) {
    case TAG_ECRAN_ACCUEIL: ecranAccueil(); break;
    case TAG_MENU_REGLAGE_HORLOGE: menuReglageHorloge(); break;
    case TAG_MENU_BASIC: menuBasic(); break;
    case TAG_MENU_VIDEOTEX: menuVideotex(); break;
    case TAG_MENU_CARTE_SD: menuCarteSD(); break;
    case TAG_MENU_PONG: menuPong(); break;
    case TAG_MENU_A_PROPOS: menuAPropos(); break;
  }
}

//////////////////////////////////////////////////////////////////////////////////////
