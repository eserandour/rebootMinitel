 //////////////////////////////////////////////////////////////////////////////////////
/*
   rebootMinitel
   Copyright 2023-2026 - Eric Sérandour
*/
   const String VERSION = "2026.11.18a";  // 00h52
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
  Compilation réussie avec Arduino 1.8.19
  Type de carte : Arduino Mega 2560 + Shield 3615 Mega (version du 27/06)
  
//////////////////////////////////////////////////////////////////////////////////////

/*
  L'électronique (non exhaustif) :
  
  * MicroSD Card Adapter de chez Catalex
  * Horloge RTC DS3231

  Le circuit :
 
    Les entrées / sorties numériques :
  ======================================
  * Buzzer sur D2
    D2 -> R = 470 ohms -> Buzzer -> GND.
 
  * La carte SD est reliée au bus SPI (bus série normalisé) de la manière suivante :
  ** SS - D53 sur l'Arduino Mega. (voir la constante CHIP_SELECT)
  ** MOSI - D51 sur l'Arduino Mega. (Master Out Slave In : Sortie de données séries)
  ** MISO - D50 sur l'Arduino Mega. (Master In Slave Out : Entrée de données séries)
  ** SCK - D52 sur l'Arduino Mega. (Serial ClocK : Pour synchroniser les échanges de données)

    Les ports de communication :
  ======================================
  * L'horloge RTC est reliée à :
  ** Bus I2C : SDA (D20 sur l'Arduino Mega)
  ** Bus I2C : SCL (D21 sur l'Arduino Mega)
  ** SQW - D34
  ** 32K - D36

  * Les ports séries UART :
  ** RX0 / TX0 - D0 / D1 pour communiquer avec le PC
  ** RX0' / TX0' - pour communiquer avec une carte à 3V3
  ** TX2 / RX2 - D16 / D17 pour communiquer avec le slot USB-A
  ** TX1 / RX1 - D18 / D19 pour communiquer avec le Minitel
   
    Bilan :
  ===========
    Sont utilisées  : D0, D1, D16, D17, D18, D19,       (Port série)
                      D2,                               (Buzzer)
                      D20, D21,                         (I2C : SDA/SCL)
                      D34, D36,                         (SQW et 32K de l'horloge RTC)
                      D50, D51, D52, D53,               (SPI pour carte SD)
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
byte ecran = TAG_ECRAN_ACCUEIL;



//////////////////////////////////////////////////////////////////////////////////////

void setup()
{
  // Liaison avec l'ordinateur. Transfert des données à 9600 bauds.
  Serial.begin(9600);  

  // Minitel
  minitel.changeSpeed(minitel.searchSpeed());
  minitel.changeSpeed(4800);
  minitel.newScreen();
  minitel.standardKeyboard();
  minitel.smallMode();

  // Liaison avec le slot USB-A du shield. Transfert des données à 9600 bauds.
  // Serial2.begin(9600);  

  // Buzzer
  pinMode(BUZZER, OUTPUT);
  
  // Initialisation de l'interface I2C.
  Wire.begin();   
  
  // Horloge RTC DS3231 (I2C)
  initHorloge();  

  // Carte SD
  // Make sure that the default chip select pin is set to output, even if you don't use it.
  pinMode(CHIP_SELECT, OUTPUT); // Chip Select en sortie.
  // See if the card is present and can be initialized. 
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
    case TAG_MENU_A_PROPOS: menuAPropos(); break;   
  }
}

//////////////////////////////////////////////////////////////////////////////////////
