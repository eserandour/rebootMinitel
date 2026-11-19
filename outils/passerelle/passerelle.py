#!/usr/bin/env python3
"""
Passerelle entre reBASIC et un dossier de l'ordinateur.

rebootMinitel - Copyright 2023-2026 - Eric Sérandour - Licence GNU GPLv3

La carte (Arduino Mega ou ESP32) est reliée à l'ordinateur par son câble USB. Depuis le
Minitel, reBASIC peut alors :
    SAVE "PC:NOM"    enregistrer le programme dans NOM.BAS, dans le dossier choisi
    LOAD "PC:NOM"    charger NOM.BAS depuis ce dossier
    MERGE "PC:NOM"   ajouter les lignes de NOM.BAS au programme en cours
    FILES "PC:"      lister les fichiers du dossier
Elle donne aussi l'heure de l'ordinateur à une carte sans horloge RTC.

Usage :
    python3 passerelle.py --dossier MES_PROGRAMMES [--port PORT] [--vitesse 9600]

Sans --port, la passerelle prend le seul port USB série branché (ou dit lesquels existent).
Il faut la bibliothèque pyserial :  python3 -m pip install pyserial

Le dialogue (voir « Fichiers sur le PC » dans Basic.ino) : des lignes de texte, celles du
protocole commencent par @.
    SAVE  : <- @SAVE NOM.BAS  -> @OK   <- lignes du programme, puis @FIN   -> @OK
    LOAD  : <- @LOAD NOM.BAS  -> @OK (ou @ERR INTROUVABLE)
            puis, pour chaque ligne : <- @SUITE  -> la ligne, ou @FIN
    FILES : <- @FILES         -> @OK, puis pour chaque fichier : <- @SUITE  -> "NOM.EXT taille" ou @FIN
    HEURE : <- @HEURE         -> @HEURE JJ/MM/AAAA hh:mm:ss   (carte sans horloge RTC)
"""

import argparse
import os
import re
import sys
import time

# Noms de fichiers 8.3, comme sur la carte SD : 1 à 8 caractères, extension de 1 à 3
NOM_83 = re.compile(r'^[A-Z0-9_-]{1,8}(\.[A-Z0-9_-]{1,3})?$', re.IGNORECASE)


def journal(texte):
    print(time.strftime('%H:%M:%S ') + texte, flush=True)


class Passerelle:
    """Répond aux lignes envoyées par reBASIC. Ne dépend pas du port série (pour les tests)."""

    def __init__(self, dossier):
        self.dossier = os.path.abspath(dossier)
        self.enregistrement = None   # (nom, lignes) pendant un SAVE
        self.envoi = None            # Lignes qui restent à envoyer (LOAD, FILES)
        self.heure_donnee = False    # Pour ne noter que la première remise à l'heure

    def chemin(self, nom):
        # Le fichier du dossier qui porte ce nom, quelle que soit la casse (sinon le nom en majuscules)
        for f in os.listdir(self.dossier):
            if f.upper() == nom and os.path.isfile(os.path.join(self.dossier, f)):
                return os.path.join(self.dossier, f)
        return os.path.join(self.dossier, nom)

    def recevoir(self, ligne):
        """Traite une ligne reçue (en octets, sans fin de ligne). Renvoie les lignes à répondre."""
        if self.enregistrement is not None and not ligne.startswith(b'@'):
            self.enregistrement[1].append(ligne)
            return []
        if not ligne.startswith(b'@'):
            # Un message de la carte (« Horloge introuvable »…) : on l'affiche
            texte = ligne.decode('utf-8', 'replace').strip()
            if texte:
                journal('Carte : ' + texte)
            return []

        mots = ligne.decode('ascii', 'replace').strip().split(' ', 1)
        demande = mots[0].upper()
        argument = mots[1].strip().upper() if len(mots) > 1 else ''

        if demande == '@HEURE':
            # L'heure de l'ordinateur, pour une carte sans horloge RTC (sans toucher à un
            # transfert en cours)
            heure = time.strftime('%d/%m/%Y %H:%M:%S').encode('ascii')
            if not self.heure_donnee:
                journal('Heure envoyée à la carte : ' + heure.decode('ascii'))
                self.heure_donnee = True
            return [b'@HEURE ' + heure]

        if demande == '@SUITE':
            if self.envoi:
                return [self.envoi.pop(0)]
            self.envoi = None
            return [b'@FIN']

        if demande == '@FIN' and self.enregistrement is not None:
            nom, lignes = self.enregistrement
            self.enregistrement = None
            return [self.ecrire(nom, lignes)]

        # Une nouvelle demande abandonne celle qui était en cours
        self.enregistrement = None
        self.envoi = None

        if demande == '@SAVE':
            if not NOM_83.match(argument):
                return [b'@ERR NOM']
            self.enregistrement = (argument, [])
            return [b'@OK']

        if demande == '@LOAD':
            if not NOM_83.match(argument):
                return [b'@ERR NOM']
            chemin = self.chemin(argument)
            if not os.path.isfile(chemin):
                journal('Introuvable : ' + argument)
                return [b'@ERR INTROUVABLE']
            try:
                with open(chemin, 'rb') as f:
                    contenu = f.read()
            except OSError as e:
                journal('Lecture impossible : %s (%s)' % (argument, e))
                return [b'@ERR LECTURE']
            if contenu.startswith(b'\xef\xbb\xbf'):  # Marque d'ordre des octets (Bloc-notes)
                contenu = contenu[3:]
            self.envoi = [l.rstrip(b'\r') for l in contenu.split(b'\n')]
            journal('Envoyé : %s (%d octets)' % (os.path.basename(chemin), len(contenu)))
            return [b'@OK']

        if demande == '@FILES':
            fichiers = []
            for f in os.listdir(self.dossier):
                chemin = os.path.join(self.dossier, f)
                if NOM_83.match(f) and os.path.isfile(chemin):
                    fichiers.append((f.upper(), os.path.getsize(chemin)))
            fichiers.sort()
            self.envoi = [('%s %d' % f).encode('ascii') for f in fichiers]
            journal('Liste envoyée (%d fichiers)' % len(fichiers))
            return [b'@OK']

        return [b'@ERR INCONNU']

    def ecrire(self, nom, lignes):
        # Même format que SAVE sur la carte SD : une ligne par ligne de programme, fin CR LF.
        # On écrit d'abord un fichier provisoire : un transfert interrompu n'abîme rien.
        chemin = self.chemin(nom)
        provisoire = chemin + '.tmp'
        try:
            with open(provisoire, 'wb') as f:
                for l in lignes:
                    f.write(l + b'\r\n')
            os.replace(provisoire, chemin)
        except OSError as e:
            journal('Écriture impossible : %s (%s)' % (nom, e))
            return b'@ERR ECRITURE'
        journal('Enregistré : %s (%d lignes)' % (os.path.basename(chemin), len(lignes)))
        return b'@OK'


def servir(flux, passerelle):
    """Lit les lignes sur le flux (port série ou autre) et répond, jusqu'à la fin du flux."""
    tampon = b''
    while True:
        donnees = flux.read(max(1, getattr(flux, 'in_waiting', 0)))
        if donnees is None:  # Fin du flux
            return
        tampon += donnees
        while b'\n' in tampon:
            ligne, tampon = tampon.split(b'\n', 1)
            for reponse in passerelle.recevoir(ligne.rstrip(b'\r')):
                flux.write(reponse + b'\n')
        if len(tampon) > 4096:  # Pas de fin de ligne : ce n'est pas reBASIC qui parle
            tampon = b''


def choisir_port(list_ports):
    ports = [p for p in list_ports.comports() if p.vid is not None]  # Ports USB seulement
    if len(ports) == 1:
        return ports[0].device
    if not ports:
        sys.exit('Aucune carte USB trouvée. Branchez la carte, ou précisez --port.')
    print('Plusieurs ports USB série : précisez celui de la carte avec --port.')
    for p in ports:
        print('  %s  (%s)' % (p.device, p.description))
    sys.exit(1)


def main():
    analyse = argparse.ArgumentParser(description='Passerelle entre reBASIC (Minitel) et un dossier de l\'ordinateur.')
    analyse.add_argument('--dossier', default='.', help='dossier des programmes (par défaut : le dossier courant)')
    analyse.add_argument('--port', help='port série de la carte (COM3, /dev/ttyACM0, /dev/cu.usbmodem…)')
    analyse.add_argument('--vitesse', type=int, default=9600, help='vitesse en bauds (9600, comme Serial.begin dans le croquis)')
    options = analyse.parse_args()

    try:
        import serial
        from serial.tools import list_ports
    except ImportError:
        sys.exit('Il faut la bibliothèque pyserial :  python3 -m pip install pyserial')

    if not os.path.isdir(options.dossier):
        sys.exit('Dossier introuvable : ' + options.dossier)
    passerelle = Passerelle(options.dossier)
    port = options.port or choisir_port(list_ports)

    class Flux:
        # Le port série, vu comme un flux : read renvoie b'' quand rien n'arrive
        def __init__(self, s): self.s = s
        @property
        def in_waiting(self): return self.s.in_waiting
        def read(self, n): return self.s.read(n)
        def write(self, b): self.s.write(b)

    journal('Dossier : ' + passerelle.dossier)
    attente_annoncee = False
    while True:
        try:
            s = serial.Serial()
            s.port = port
            s.baudrate = options.vitesse
            s.timeout = 0.1
            # DTR et RTS au repos : évite, quand c'est possible, que la carte redémarre à
            # l'ouverture du port (l'ESP32 surtout ; l'Arduino Mega redémarre souvent quand même)
            s.dtr = False
            s.rts = False
            s.open()
        except serial.SerialException as e:
            if not attente_annoncee:
                journal('Port %s indisponible (%s). Nouvel essai toutes les 2 secondes…' % (port, e))
                attente_annoncee = True
            time.sleep(2)
            continue
        attente_annoncee = False
        journal('Port %s ouvert à %d bauds. Ctrl-C pour arrêter.' % (port, options.vitesse))
        try:
            servir(Flux(s), passerelle)
        except serial.SerialException:
            journal('Carte débranchée.')
        finally:
            s.close()


if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print()
