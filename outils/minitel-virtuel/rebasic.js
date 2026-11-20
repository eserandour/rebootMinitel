/*!
 * rebasic.js — reBASIC dans une page web, sur l'écran d'un Minitel virtuel
 *
 * reBASIC est le BASIC de rebootMinitel (https://github.com/eserandour/rebootMinitel).
 * Cette bibliothèque en est un portage fidèle en JavaScript : l'interpréteur reprend
 * Basic.ino ligne à ligne (mêmes instructions, mêmes messages, mêmes limites, nombres
 * en float 32 bits), et l'écran décode le flux Videotex envoyé au Minitel avec les
 * règles du STUM 1B (attributs de zone, double hauteur, semi-graphiques, défilement).
 *
 * Utilisation :
 *   <div id="minitel"></div>
 *   <script src="rebasic.js"></script>
 *   <script>
 *     const m = ReBASIC.creer(document.getElementById('minitel'));
 *     m.charger('10 PRINT "Bonjour"');
 *     m.lancer();
 *   </script>
 *
 * Fait partie de rebootMinitel. Copyright 2023-2026 - Eric Sérandour.
 * Logiciel libre sous licence GNU GPLv3.
 */
(function (racine, fabrique) {
  if (typeof module === 'object' && module.exports) module.exports = fabrique();
  else racine.ReBASIC = fabrique();
})(typeof self !== 'undefined' ? self : this, function () {
  'use strict';

  const VERSION = '2026.11.20a';  // Celle de rebootMinitel

  ////////////////////////////////////////////////////////////////////////////////////
  // Constantes de Minitel1B_Hard
  ////////////////////////////////////////////////////////////////////////////////////

  const ENVOI = 0x1341, RETOUR = 0x1342, REPETITION = 0x1343, GUIDE = 0x1344,
    ANNULATION = 0x1345, SOMMAIRE = 0x1346, CORRECTION = 0x1347, SUITE = 0x1348;
  const FLECHE_HAUT = 0x1B5B41, FLECHE_BAS = 0x1B5B42, FLECHE_DROITE = 0x1B5B43, FLECHE_GAUCHE = 0x1B5B44;

  const CARACTERE_NOIR = 0x40, CARACTERE_BLANC = 0x47, CLIGNOTEMENT = 0x48, FIXE = 0x49,
    GRANDEUR_NORMALE = 0x4C, DOUBLE_HAUTEUR = 0x4D, DOUBLE_GRANDEUR = 0x4F, FOND_NOIR = 0x50,
    FIN_LIGNAGE = 0x59, DEBUT_LIGNAGE = 0x5A, FOND_NORMAL = 0x5C, INVERSION_FOND = 0x5D;

  const TOUCHES = {
    ENVOI, RETOUR, REPETITION, GUIDE, ANNULATION, SOMMAIRE, CORRECTION, SUITE,
    CONNEXION: 0x1349, HAUT: FLECHE_HAUT, BAS: FLECHE_BAS, DROITE: FLECHE_DROITE, GAUCHE: FLECHE_GAUCHE
  };

  // Caractères du jeu G2 du Minitel (précédés de SS2, 0x19)
  const G2 = {
    '\u00a3': [0x23], '\u00a7': [0x27], '\u2190': [0x2C], '\u2191': [0x2D], '\u2192': [0x2E], '\u2193': [0x2F],
    '\u00b0': [0x30], '\u00b1': [0x31], '\u00f7': [0x38], '\u00bc': [0x3C], '\u00bd': [0x3D], '\u00be': [0x3E],
    '\u0152': [0x6A], '\u0153': [0x7A], '\u00df': [0x7B],
    '\u00e0': [0x41, 0x61], '\u00e8': [0x41, 0x65], '\u00f9': [0x41, 0x75],
    '\u00e9': [0x42, 0x65],
    '\u00e2': [0x43, 0x61], '\u00ea': [0x43, 0x65], '\u00ee': [0x43, 0x69], '\u00f4': [0x43, 0x6F], '\u00fb': [0x43, 0x75],
    '\u00e4': [0x48, 0x61], '\u00eb': [0x48, 0x65], '\u00ef': [0x48, 0x69], '\u00f6': [0x48, 0x6F], '\u00fc': [0x48, 0x75],
    '\u00e7': [0x4B, 0x63]
  };
  // Ce que Minitel1B_Hard (print) envoie au Minitel pour chaque caractère UTF-8 qu'il connaît.
  // Les majuscules accentuées deviennent la lettre seule (le Minitel n'en a pas, STUM p. 90).
  // Un caractère absent de cette table n'est pas envoyé du tout.
  const HARD = {
    '\u00a3': [0x19, 0x23],
    '\u00a7': [0x19, 0x27],
    '\u00b0': [0x19, 0x30],
    '\u00b1': [0x19, 0x31],
    '\u00bc': [0x19, 0x3C],
    '\u00bd': [0x19, 0x3D],
    '\u00be': [0x19, 0x3E],
    '\u00c0': [0x0F, 0x41],
    '\u00c2': [0x0F, 0x41],
    '\u00c4': [0x0F, 0x41],
    '\u00c7': [0x0F, 0x43],
    '\u00c8': [0x0F, 0x45],
    '\u00c9': [0x0F, 0x45],
    '\u00ca': [0x0F, 0x45],
    '\u00cb': [0x0F, 0x45],
    '\u00ce': [0x0F, 0x49],
    '\u00cf': [0x0F, 0x49],
    '\u00d4': [0x0F, 0x4F],
    '\u00d6': [0x0F, 0x4F],
    '\u00d9': [0x0F, 0x55],
    '\u00db': [0x0F, 0x55],
    '\u00dc': [0x0F, 0x55],
    '\u00e0': [0x19, 0x41, 0x61],
    '\u00e2': [0x19, 0x43, 0x61],
    '\u00e4': [0x19, 0x48, 0x61],
    '\u00e7': [0x19, 0x4B, 0x63],
    '\u00e8': [0x19, 0x41, 0x65],
    '\u00e9': [0x19, 0x42, 0x65],
    '\u00ea': [0x19, 0x43, 0x65],
    '\u00eb': [0x19, 0x48, 0x65],
    '\u00ee': [0x19, 0x43, 0x69],
    '\u00ef': [0x19, 0x48, 0x69],
    '\u00f4': [0x19, 0x43, 0x6F],
    '\u00f6': [0x19, 0x48, 0x6F],
    '\u00f7': [0x19, 0x38],
    '\u00f9': [0x19, 0x41, 0x75],
    '\u00fb': [0x19, 0x43, 0x75],
    '\u00fc': [0x19, 0x48, 0x75],
    '\u0152': [0x19, 0x6A],
    '\u0153': [0x19, 0x7A],
    '\u03b2': [0x19, 0x7B],
    '\u2014': [0x60],
    '\u2190': [0x19, 0x2C],
    '\u2191': [0x5E],
    '\u2192': [0x19, 0x2E],
    '\u2193': [0x19, 0x2F]
  };

  const G2_SIMPLE = {}, G2_COMPOSE = {};
  for (const ch in G2) {
    const s = G2[ch];
    if (s.length === 1) G2_SIMPLE[s[0]] = ch;
    else G2_COMPOSE[s[0] * 256 + s[1]] = ch;
  }
  const DIACRITIQUES = [0x41, 0x42, 0x43, 0x48, 0x4B];

  ////////////////////////////////////////////////////////////////////////////////////
  // Outils
  ////////////////////////////////////////////////////////////////////////////////////

  const F = Math.fround;  // Les nombres de reBASIC sont des float (32 bits)

  function u8c(ch) {
    // Nombre d'octets UTF-8 d'un caractère
    const c = ch.codePointAt(0);
    return c < 0x80 ? 1 : c < 0x800 ? 2 : c < 0x10000 ? 3 : 4;
  }

  function u8len(s) {
    let n = 0;
    for (const ch of s) n += u8c(ch);
    return n;
  }

  // Les mots-clés « tokenisés » (basTokNoms dans Basic.ino). Sur la carte, chaque mot-clé
  // du programme est rangé sous la forme d'un code d'un octet (les TOK_UN premiers) ou de
  // deux octets (les suivants). Ici, le programme reste en texte (l'interpréteur le lit
  // exactement comme Basic.ino lit le texte tokenisé), mais sa taille est comptée comme
  // sur la carte : tailleTokenisee() reprend basTokenize.
  const TOK_NOMS = [
    'PRINT', 'INPUT', 'IF', 'THEN', 'ELSE', 'GOTO', 'GOSUB', 'RETURN',
    'FOR', 'TO', 'STEP', 'NEXT', 'WHILE', 'WEND', 'DATA', 'READ',
    'RESTORE', 'DIM', 'REM', 'END', 'STOP', 'CLS', 'LOCATE', 'PAUSE',
    'SOUND', 'BEEP', 'PLOT', 'LINE', 'BOX', 'CIRCLE', 'COLOR', 'PAPER',
    'INVERSE', 'FLASH', 'UNDERLINE', 'SIZE', 'AND', 'OR', 'NOT', 'MOD',
    'INT', 'RND', 'LEN', 'CHR$', 'STR$', 'LEFT$', 'RIGHT$', 'MID$',
    'INKEY$', 'KEY', 'POINT', 'TIMER', 'TIME$', 'DATE$', 'ASC', 'VAL',
    'INSTR', 'TAB', 'ABS', 'SQR', 'SIN', 'COS', 'ON', 'VDT',
    'CHAIN', 'SGN', 'TAN', 'ATN', 'LOG', 'EXP', 'HOUR', 'MINUTE',
    'SECOND', 'DAY', 'MONTH', 'YEAR', 'DREAD', 'AREAD', 'EOF', 'HEX$',
    'UPPER$', 'LOWER$', 'OPEN', 'CLOSE', 'OUTPUT', 'APPEND', 'PINMODE', 'DWRITE',
    'AWRITE', 'LET', 'RUN', 'LIST', 'NEW', 'MEM', 'RENUM', 'AIDE',
    'HELP', 'SAVE', 'LOAD', 'MERGE', 'FILES', 'KILL', 'EDIT'
  ];
  const TOK_UN = 64;
  const estLettre = (ch) => ch !== undefined && ch >= 'A' && ch <= 'Z';

  function tailleTokenisee(s) {
    // Nombre d'octets du texte s (en majuscules) une fois tokenisé, comme basTokenize
    let n = 0, i = 0;
    let brut = false, donnees = false, guillemets = false;
    while (i < s.length) {
      const ch = s[i];
      const c = s.codePointAt(i);
      if (c >= 0x80) { const l = String.fromCodePoint(c); n += u8c(l); i += l.length; continue; }
      if (ch === '"') guillemets = !guillemets;
      if (guillemets || brut || ch === '"') { n++; i++; continue; }
      if (donnees) { if (ch === ':') donnees = false; n++; i++; continue; }
      if (ch === "'") { brut = true; n++; i++; continue; }
      if (!estLettre(ch)) { n++; i++; continue; }
      // Début d'un mot
      if (s.startsWith('REM', i)) brut = true;
      else if (s.startsWith('DATA', i)) donnees = true;
      let id = -1;
      for (let k = 0; k < TOK_NOMS.length; k++) {
        const nom = TOK_NOMS[k];
        if (!s.startsWith(nom, i)) continue;
        if (estLettre(nom[nom.length - 1]) && estLettre(s[i + nom.length])) continue;
        id = k;
        break;
      }
      if (id < 0) { while (estLettre(s[i])) { n++; i++; } continue; }
      n += id < TOK_UN ? 1 : 2;
      i += TOK_NOMS[id].length;
    }
    return n;
  }

  function u8tronquer(s, max) {
    // Les premiers caractères de s qui tiennent dans max octets
    let n = 0, out = '';
    for (const ch of s) {
      const l = u8c(ch);
      if (n + l > max) break;
      out += ch;
      n += l;
    }
    return out;
  }

  const encodeur = typeof TextEncoder !== 'undefined' ? new TextEncoder() : null;
  const decodeur = typeof TextDecoder !== 'undefined' ? new TextDecoder() : null;
  const versOctets = (s) => encodeur.encode(s);
  const depuisOctets = (o) => decodeur.decode(o);

  const maintenant = (typeof performance !== 'undefined' && performance.now)
    ? () => performance.now() : () => Date.now();

  ////////////////////////////////////////////////////////////////////////////////////
  // L'écran du Minitel : décodage du flux Videotex
  ////////////////////////////////////////////////////////////////////////////////////
  //
  // Mêmes règles que le simulateur de rebootMinitel (STUM 1B, partie 2, chapitre 2) :
  // - couleur, clignotement, inversion et taille s'appliquent aux caractères qui suivent ;
  // - le fond et le soulignement sont des attributs de zone : ils restent latents jusqu'au
  //   prochain espace (délimiteur) ; un caractère semi-graphique valide aussi le fond, et
  //   délimite lui aussi une zone ;
  // - une case qui n'est pas un délimiteur (lettre, espace sans changement, case jamais
  //   écrite) montre le fond du dernier délimiteur à sa gauche, au moment de l'affichage :
  //   récrire un délimiteur change le fond des cases qui le suivent, jusqu'au suivant
  //   (constaté sur un Minitel 1B Alcatel) ;
  // - le passage en semi-graphique annule la taille, l'inversion et le soulignement ;
  // - la double hauteur est dessinée vers le haut, et ignorée sur la première rangée ;
  // - mode rouleau : l'écran défile quand on descend sous la 24e rangée.
  // Pour les pages Vidéotex (menu 2), il comprend aussi la répétition (REP), la rangée 0
  // (ignorée : c'est la ligne de service du Minitel) et les commandes PRO1 à PRO3, que
  // reBASIC n'envoie jamais.

  function caseVide() {
    return {
      car: ' ', couleur: 7, fond: 0, inverse: false, clignote: false, souligne: false,
      graphique: false, points: 0, taille: 0, couverte: false, zoneFond: 0, zoneSouligne: false
    };
  }

  class Ecran {
    constructor() {
      this.cases = [];
      for (let i = 0; i < 24; i++) {
        const l = [];
        for (let j = 0; j < 40; j++) l.push(caseVide());
        this.cases.push(l);
      }
      this.r = 0;
      this.c = 0;
      this.curseur = false;
      this.etat = 0;
      this.csi = '';
      this.memo = 0;
      this.modifie = true;
      this.surBip = null;
      this.initAttributs();
    }

    initAttributs() {
      this.couleur = 7; this.taille = 0;
      this.inverse = false; this.clignote = false; this.modeGraphique = false;
      this.fondLatent = 0; this.fondZone = 0;
      this.soulLatent = false; this.soulZone = false;
      this.delimiteurAttendu = false;
    }

    effacer() {
      for (const l of this.cases) for (let j = 0; j < 40; j++) l[j] = caseVide();
      this.r = 0; this.c = 0;
      this.rangee0 = false;
      this.initAttributs();
      this.modifie = true;
    }

    nouvelleRangee() { this.fondZone = 0; this.soulZone = false; }

    zoneIci() {
      // Après un déplacement : les attributs de zone sont ceux de la case de gauche
      if (this.c > 0) {
        const x = this.cases[this.r][this.c - 1];
        this.fondZone = x.zoneFond;
        this.soulZone = x.zoneSouligne;
      }
      else this.nouvelleRangee();
    }

    propager(r, c) {
      // Le fond d'une case qui n'est pas un délimiteur est celui du dernier délimiteur à
      // sa gauche : on le recalcule pour la case écrite et celles qui la suivent
      const l = this.cases[r];
      const g = c > 0 ? l[c - 1] : null;
      const z = l[c].delim ? l[c].fond : (g ? g.zoneFond : 0);
      for (let j = c; j < 40; j++) {
        if (j > c && l[j].delim) break;
        l[j].fond = z; l[j].zoneFond = z;
      }
    }

    defiler() {
      this.cases.shift();
      const l = [];
      for (let j = 0; j < 40; j++) l.push(caseVide());
      this.cases.push(l);
      this.r = 23;
      this.modifie = true;
    }

    descendre() { if (++this.r >= 24) this.defiler(); this.nouvelleRangee(); }

    avancer(n) { this.c += n; if (this.c >= 40) { this.c -= 40; this.descendre(); } }

    poser(car, graphique, points) {
      if (this.rangee0) return;  // La rangée 0 n'est pas dessinée
      this.dernier = [car, graphique, points];  // Pour la répétition (REP)
      let delimiteur = false;
      if (graphique) {
        this.fondZone = this.fondLatent;
        this.delimiteurAttendu = false;
        delimiteur = true;
      }
      else if (car === ' ' && (this.delimiteurAttendu || this.fondLatent !== this.fondZone || this.soulLatent !== this.soulZone)) {
        this.fondZone = this.fondLatent;
        this.soulZone = this.soulLatent;
        this.delimiteurAttendu = false;
        delimiteur = true;
      }
      const t = graphique ? 0 : this.taille;
      const larg = (t & 2) ? 2 : 1, haut = (t & 1) ? 2 : 1;
      for (let dy = 0; dy < haut; dy++) {
        for (let dx = 0; dx < larg; dx++) {
          const rr = this.r - dy, cc = this.c + dx;
          if (rr < 0 || cc > 39) continue;
          const x = caseVide();
          x.couverte = (dx > 0 || dy > 0);
          x.car = car; x.couleur = this.couleur; x.fond = this.fondZone;
          x.inverse = graphique ? false : this.inverse;
          x.clignote = this.clignote;
          x.souligne = (graphique || delimiteur) ? false : this.soulZone;
          x.graphique = graphique; x.points = points; x.taille = t;
          x.zoneFond = this.fondZone; x.zoneSouligne = this.soulZone;
          x.delim = delimiteur && dx === 0 && dy === 0;
          this.cases[rr][cc] = x;
          this.propager(rr, cc);
        }
      }
      this.avancer(larg);
      this.modifie = true;
    }

    attribut(a) {
      if (a >= 0x40 && a <= 0x47) this.couleur = a - 0x40;
      else if (a === 0x48) this.clignote = true;
      else if (a === 0x49) this.clignote = false;
      else if (a >= 0x4C && a <= 0x4F) {
        if (this.modeGraphique) return;
        if ((a === 0x4D || a === 0x4F) && this.r + 1 <= 1) return;
        this.taille = a - 0x4C;
      }
      else if (a >= 0x50 && a <= 0x57) { this.fondLatent = a - 0x50; this.delimiteurAttendu = true; }
      else if (a === 0x59) { this.soulLatent = false; this.delimiteurAttendu = true; }
      else if (a === 0x5A) { if (!this.modeGraphique) { this.soulLatent = true; this.delimiteurAttendu = true; } }
      else if (a === 0x5C) { if (!this.modeGraphique) this.inverse = false; }
      else if (a === 0x5D) { if (!this.modeGraphique) this.inverse = true; }
    }

    modeSemiGraphique() {
      this.modeGraphique = true; this.taille = 0; this.inverse = false;
      this.soulLatent = false; this.soulZone = false;
    }

    finCsi(b) {
      const n = parseInt(this.csi, 10) || 1;
      if (b === 0x41) { this.r = Math.max(0, this.r - n); this.zoneIci(); }
      else if (b === 0x42) { this.r = Math.min(23, this.r + n); this.zoneIci(); }
      else if (b === 0x43) { this.c = Math.min(39, this.c + n); this.zoneIci(); }
      else if (b === 0x44) { this.c = Math.max(0, this.c - n); this.zoneIci(); }
      else if (b === 0x4B) { this.effacerFinRangee(); }
    }

    effacerFinRangee() {
      for (let j = this.c; j < 40; j++) this.cases[this.r][j] = caseVide();
      this.modifie = true;
    }

    recevoir(b) {
      // Reçoit un octet du flux envoyé au Minitel
      switch (this.etat) {
        case 1:  // Après ESC
          this.etat = 0;
          if (b === 0x5B) { this.etat = 2; this.csi = ''; return; }
          // PRO1, PRO2, PRO3 (réglages du Minitel) et masquage : on saute leurs paramètres
          if (b === 0x39 || b === 0x3A || b === 0x3B) { this.saut = b - 0x38; this.etat = 8; return; }
          if (b === 0x23) { this.saut = 2; this.etat = 8; return; }
          this.attribut(b);
          return;
        case 2:  // Séquence CSI
          if (b >= 0x30 && b <= 0x3F) { this.csi += String.fromCharCode(b); return; }
          this.etat = 0;
          this.finCsi(b);
          return;
        case 3:  // US : rangée
          this.memo = b; this.etat = 4; return;
        case 4:  // US : colonne
          this.etat = 0;
          if (this.memo === 0x40) {  // Rangée 0 : on garde la position, jusqu'au LF qui en sort
            if (!this.rangee0) this.avant0 = [this.r, this.c];
            this.rangee0 = true;
            return;
          }
          this.rangee0 = false;
          this.r = Math.min(23, Math.max(0, this.memo - 0x41));
          this.c = Math.min(39, Math.max(0, b - 0x41));
          this.zoneIci();
          return;
        case 5:  // SS2 : caractère du jeu G2
          if (DIACRITIQUES.indexOf(b) >= 0) { this.memo = b; this.etat = 6; return; }
          this.etat = 0;
          if (G2_SIMPLE[b]) this.poser(G2_SIMPLE[b], false, 0);
          return;
        case 6:  // SS2 : lettre accentuée
          this.etat = 0;
          this.poser(G2_COMPOSE[this.memo * 256 + b] || String.fromCharCode(b), false, 0);
          return;
        case 7:  // REP : le dernier caractère, répété (b - 0x40) fois
          this.etat = 0;
          if (this.dernier) for (let i = 0; i < (b & 0x3F); i++) this.poser(...this.dernier);
          return;
        case 8:  // Paramètres d'une commande que l'on saute
          if (--this.saut <= 0) this.etat = 0;
          return;
      }
      switch (b) {
        case 0x1B: this.etat = 1; return;
        case 0x1F: this.etat = 3; return;
        case 0x19: this.etat = 5; return;
        case 0x0D: this.c = 0; this.zoneIci(); this.modifie = true; return;
        case 0x0A:
          if (this.rangee0) { this.rangee0 = false; [this.r, this.c] = this.avant0; this.zoneIci(); return; }
          this.descendre(); this.modifie = true; return;
        case 0x12: this.etat = 7; return;
        case 0x08: if (this.c > 0) this.c--; this.zoneIci(); this.modifie = true; return;
        case 0x09: this.c = Math.min(39, this.c + 1); this.zoneIci(); this.modifie = true; return;
        case 0x0B: this.r = Math.max(0, this.r - 1); this.zoneIci(); this.modifie = true; return;
        case 0x0C: this.effacer(); return;
        case 0x0E: this.modeSemiGraphique(); return;
        case 0x0F: this.modeGraphique = false; return;
        case 0x1E: this.r = 0; this.c = 0; this.zoneIci(); this.modifie = true; return;
        case 0x18: if (!this.rangee0) this.effacerFinRangee(); return;
        case 0x11: this.curseur = true; this.modifie = true; return;
        case 0x14: this.curseur = false; this.modifie = true; return;
        case 0x07: if (this.surBip) this.surBip(); return;
      }
      if (b < 0x20) return;
      if (this.modeGraphique) {
        // Caractère semi-graphique : on range ses 6 points dans l'ordre de reBASIC
        // (bit 5 en haut à gauche ... bit 0 en bas à droite)
        if (b < 0x20 || (b >= 0x40 && b < 0x60)) return;
        const p = ((b & 1) ? 32 : 0) | ((b & 2) ? 16 : 0) | ((b & 4) ? 8 : 0) |
          ((b & 8) ? 4 : 0) | ((b & 16) ? 2 : 0) | ((b & 64) ? 1 : 0);
        this.poser(' ', true, b === 0x7F ? 63 : p);
        return;
      }
      if (b === 0x7F) return;
      this.poser(String.fromCharCode(b), false, 0);
    }

    texte() {
      // L'écran en texte (24 lignes), comme le simulateur : # pour un point allumé
      const lignes = [];
      for (let i = 0; i < 24; i++) {
        let s = '';
        for (let j = 0; j < 40; j++) {
          const x = this.cases[i][j];
          if (x.couverte) s += ' ';
          else if (x.graphique) s += x.points ? '#' : ' ';
          else s += x.car;
        }
        lignes.push(s.replace(/\s+$/, ''));
      }
      return lignes;
    }
  }

  ////////////////////////////////////////////////////////////////////////////////////
  // L'interpréteur reBASIC (portage de Basic.ino)
  ////////////////////////////////////////////////////////////////////////////////////

  // Ce que l'interpréteur attend quand il rend la main (yield)
  const ATTENTE_CLE = { t: 'cle' };
  const ATTENTE_FLUX = { t: 'flux' };
  const TRANCHE = { t: 'tranche' };

  // Erreurs
  const E_SYNTAXE = 1, E_DIVZERO = 2, E_LIGNE = 3, E_RETURN = 4, E_NEXT = 5, E_MEMOIRE = 6,
    E_TYPE = 7, E_INDICE = 8, E_FICHIER = 9, E_IMBRIC = 10, E_VALEUR = 11, E_BREAK = 12,
    E_DATA = 13, E_WEND = 14, E_WHILE = 15, E_NONOUVERT = 16, E_FINFICH = 17, E_INTROUV = 18,
    E_DEJA = 19, E_BROCHE = 20, E_HORLOGE = 21, E_LONGUE = 22, E_PC = 23;
  const MESSAGES = {
    [E_SYNTAXE]: 'Erreur de syntaxe', [E_DIVZERO]: 'Division par z\u00e9ro', [E_RETURN]: 'RETURN sans GOSUB',
    [E_NEXT]: 'NEXT sans FOR', [E_MEMOIRE]: 'M\u00e9moire pleine', [E_TYPE]: 'Type incompatible',
    [E_INDICE]: 'Indice hors limites', [E_FICHIER]: 'Erreur de fichier', [E_IMBRIC]: "Trop d'imbrications",
    [E_VALEUR]: 'Valeur incorrecte', [E_BREAK]: 'Interruption', [E_DATA]: 'Plus de DATA',
    [E_WEND]: 'WEND sans WHILE', [E_WHILE]: 'WHILE sans WEND', [E_NONOUVERT]: 'Fichier non ouvert',
    [E_FINFICH]: 'Fin de fichier', [E_INTROUV]: 'Fichier introuvable', [E_DEJA]: 'Fichier d\u00e9j\u00e0 ouvert',
    [E_BROCHE]: 'Broche interdite', [E_HORLOGE]: 'Horloge absente', [E_LONGUE]: 'Ligne trop longue',
    [E_PC]: 'PC absent'
  };

  // Résultats de la saisie d'une ligne
  const K_OK = 0, K_QUITTER = 1, K_ANNULER = 2, K_AIDE = 3, K_COMMANDE = 4;

  // Bits du style
  const ST_INVERSE = 0x01, ST_FLASH = 0x02, ST_SOULIGNE = 0x04, ST_HAUT = 0x10, ST_LARGE = 0x20;

  function creerInterpreteur(env) {
    const esp = (env.carte === 'esp32');
    // Le programme et les tableaux se partagent MEM_SIZE octets (BAS_MEM_SIZE) : le programme
    // part du début, les tableaux de la fin (4 octets par élément)
    const MEM_SIZE = esp ? 20384 : 2304;
    const MEM_FLOATS = MEM_SIZE / 4;
    const STR_MAX = esp ? 78 : 32;
    const LINE_MAX = 78;
    const TEXT_MAX = LINE_MAX + 24;
    const GOSUB_MAX = 12, FOR_MAX = 8, WHILE_MAX = 6, FILE_MAX = 2;
    const DEPTH_MAX = esp ? 8 : 6;
    const GW = 80, GH = 72;

    // Le programme : lignes triées { n: numéro, t: texte }
    let prog = [];
    let progLen = 0;

    // Les variables
    const vars = new Float32Array(26);
    const strs = new Array(26).fill('');
    const arr = new Float32Array(MEM_FLOATS);  // Les éléments des tableaux sont à la fin
    const arrBase = new Int32Array(26).fill(-1);
    const arrDim = new Int32Array(26);
    let arrUsed = 0;
    let chaine = false;  // CHAIN en cours : les tableaux doivent être gardés
    const libre = () => MEM_SIZE - progLen - 4 * arrUsed;

    // Les piles (ligne : indice dans prog, -1 pour la ligne tapée au clavier)
    let gs = [], fors = [], whs = [];

    // DATA / READ (dataLigne -1 : début du programme)
    let dataLigne = -1, dataPos = 0, dataIn = false;

    // Fichiers de données
    const fichiers = [null, null];

    // L'exécution
    let ligneDirecte = '';   // Ligne tapée au clavier
    let inBuf = '';          // Ligne à corriger (EDIT)
    let T = '';              // Texte en cours d'exécution
    let P = 0;               // Position dans T
    let cur = -1;            // Ligne en cours (-1 : ligne tapée au clavier)
    let err = 0, stop = false, jumped = false, attr = false;
    let ink = 7, paper = 0, style = 0;
    let editPending = false, depth = 0, col = 0, key = 0;
    const gfx = new Uint8Array(GW * GH);
    let timer0 = 0, errLigne = 0;
    let arretExterne = false, arretSilencieux = false, enCours = false;
    let rythmeT = 0, trancheT = 0;

    const texteDe = (l) => (l < 0 ? ligneDirecte : prog[l].t);
    function aller(l, p) { cur = l; T = texteDe(l); P = p; }

    //////////////////////////////////////////////////////////////////////////////
    // Affichage

    const wb = (b) => env.octet(b);

    function avance() {
      col += (style & ST_LARGE) ? 2 : 1;
      if (col >= 40) col -= 40;
    }

    function esc(a) { wb(0x1B); wb(a); }

    function attributs(a) {
      // Comme minitel.attributs() : la double hauteur descend le curseur d'une rangée
      wb(0x1B); wb(a);
      if (a === DOUBLE_HAUTEUR || a === DOUBLE_GRANDEUR) wb(0x0A);
    }

    function moveXY(x, y) { wb(0x1F); wb(0x40 + y); wb(0x40 + x); }

    function csi(n, f) {
      wb(0x1B); wb(0x5B);
      for (const ch of String(n)) wb(ch.charCodeAt(0));
      wb(f);
    }

    function moveUp(n) { if (n <= 4) { for (let i = 0; i < n; i++) wb(0x0B); } else csi(n, 0x41); }
    function moveRight(n) { if (n <= 4) { for (let i = 0; i < n; i++) wb(0x09); } else csi(n, 0x43); }
    function moveLeft(n) { if (n <= 4) { for (let i = 0; i < n; i++) wb(0x08); } else csi(n, 0x44); }
    function clearLineFromCursor() { wb(0x1B); wb(0x5B); wb(0x4B); }
    function newScreen() { wb(0x0C); }
    function bip() { wb(0x07); }

    function restoreAttr() {
      if (style & ST_INVERSE) esc(INVERSION_FOND);
      if (style & ST_SOULIGNE) esc(DEBUT_LIGNAGE);
      const taille = (style & (ST_HAUT | ST_LARGE)) >> 4;
      if (taille) esc(GRANDEUR_NORMALE + taille);
    }

    function resetAttr() {
      attributs(CARACTERE_BLANC);
      attributs(FOND_NORMAL);
      attributs(FIXE);
      attributs(FIN_LIGNAGE);
      attributs(GRANDEUR_NORMALE);
      attributs(FOND_NOIR);
      ink = 7; paper = 0; style = 0;
    }

    function newLine() { wb(0x0D); wb(0x0A); col = 0; }

    function puts(s) {
      for (const ch of s) {
        const c = ch.codePointAt(0);
        if (c === 10) newLine();
        else if (c < 0x20) {
          wb(c);
          if (c === 0x0D) col = 0;
          if (c === 0x08 && col > 0) col--;
        }
        else if (c < 0x80) { wb(c); avance(); }
        else {
          // Comme basPuts : les caractères dont le premier octet UTF-8 est C2, C3, C5, CE ou E2
          // passent par minitel.print() (table HARD), les autres deviennent « ? »
          const premier = (c >= 0x80 && c < 0xC0) || (c >= 0xC0 && c < 0x100) || (c >= 0x140 && c < 0x180) ||
            (c >= 0x380 && c < 0x3C0) || (c >= 0x2000 && c < 0x3000);
          if (!premier) wb(0x3F);
          else if (HARD[ch]) for (const b of HARD[ch]) wb(b);
          avance();
        }
      }
    }

    function numToStr(v) {
      v = F(v);
      if (v !== v) return 'NAN';
      let out = '';
      if (v < 0) { out = '-'; v = -v; }
      if (v > 3.4e38) return out + 'INF';
      if (v === 0) return out + '0';
      let expo = 0;
      const sci = (v >= 1e9 || v < 1e-4);
      if (sci) {
        while (v >= 10) { v = F(v / 10); expo++; }
        while (v < 1) { v = F(v * 10); expo--; }
      }
      let ip = Math.trunc(v);
      const fp = F(v - ip);
      let chiffres = 0;
      for (let t = ip; t > 0; t = Math.floor(t / 10)) chiffres++;
      let dec = 7 - chiffres;
      if (dec > 6) dec = 6;
      if (dec < 0) dec = 0;
      const echelle = Math.pow(10, dec);
      let f = Math.trunc(F(F(fp * echelle) + 0.5));
      if (f >= echelle) { ip++; f -= echelle; }
      if (sci && ip >= 10) { ip = 1; f = 0; expo++; }
      out += String(ip);
      if (f > 0) out += '.' + String(f).padStart(dec, '0').replace(/0+$/, '');
      if (sci) out += 'E' + (expo < 0 ? '-' : '') + Math.abs(expo);
      return out;
    }

    const printNum = (v) => puts(numToStr(v));

    function showErrorLine(texte, pos) {
      if (cur >= 0) { printNum(prog[cur].n); puts(' '); }
      puts(texte.slice(0, pos));
      const c = pos >= texte.length ? ' ' : texte[pos];
      attributs(INVERSION_FOND);
      puts(c);
      attributs(FOND_NORMAL);
      if (pos < texte.length) puts(texte.slice(pos + 1));
      if (col !== 0) newLine();
    }

    function printError() {
      const pos = P;
      resetAttr();
      if (col !== 0) newLine();
      if (err === E_LIGNE) {
        puts('Ligne ');
        if (errLigne > 0) { printNum(errLigne); puts(' '); }
        puts('inexistante');
      }
      else puts(MESSAGES[err] || 'Erreur');
      if (cur >= 0) { puts(' en ligne '); printNum(prog[cur].n); }
      newLine();
      if (err !== E_BREAK) {
        if (pos >= 0 && pos <= T.length) showErrorLine(T, pos);
        bip();
      }
      errLigne = 0;
    }

    function keyName(nom) {
      attributs(INVERSION_FOND);
      puts(nom);
      attributs(FOND_NORMAL);
    }

    const HELP_PAGES = 4;
    const AIDE = [
      ['Commandes :', ' RUN LIST [n][-m] [/P] NEW MEM AIDE', ' EDIT n  RENUM [d\u00e9but][,pas]',
        ' SAVE "NOM" LOAD "NOM" MERGE "NOM"', ' FILES KILL "NOM"', ' CHAIN "NOM" (garde les variables)', ' "PC:NOM" FILES "PC:" : sur le PC', '',
        'Instructions :', ' PRINT (ou ?) avec ; , TAB(n)', ' INPUT ["texte";] v[,v...]',
        ' [LET] v = expression', ' IF cond THEN ... [ELSE ...]', ' GOTO n  GOSUB n  RETURN',
        ' ON x GOTO n1,n2...  ON x GOSUB ...', ' FOR v=a TO b [STEP c] ... NEXT [v]',
        ' WHILE cond ... WEND', ' DATA  READ v[,v...]  RESTORE [n]', ' DIM A(n)  REM  END  STOP'],
      ['Ecran (couleurs 0 \u00e0 7) :', ' CLS  LOCATE col,rang', ' VDT "NOM" : page Vid\u00e9otex NOM.VDT', ' COLOR n  PAPER n (fond)',
        ' INVERSE n  FLASH n  UNDERLINE n', ' SIZE n (0 normale, 1 double hauteur,',
        '  2 double largeur, 3 double taille)', ' PAPER et UNDERLINE commencent au',
        ' premier espace qui suit.', '', 'Son :', ' BEEP  SOUND f,ms  PAUSE ms', '',
        'Dessin (80 x 72 points) :', ' PLOT x,y[,c]', ' LINE x1,y1,x2,y2[,c]',
        ' BOX x1,y1,x2,y2[,c[,p]]', ' CIRCLE x,y,r[,c[,p]]', ' (c=0 efface, p=1 plein)  POINT(x,y)'],
      ['Fonctions :', ' ABS INT SGN SQR SIN COS TAN ATN', ' LOG EXP RND(n) POINT(x,y)',
        ' LEN VAL ASC CHR$ STR$ HEX$', ' LEFT$ RIGHT$ MID$ INSTR', ' UPPER$ LOWER$ INKEY$ KEY',
        ' TIMER (ms depuis le RUN)', ' TIME$ DATE$ HOUR MINUTE SECOND', ' DAY MONTH YEAR', '',
        'Op\u00e9rateurs : + - * / ^ MOD', ' = <> < > <= >= AND OR NOT', '',
        'Codes de KEY : ENVOI 4929', ' RETOUR 4930  REPETITION 4931', ' GUIDE 4932  CORRECTION 4935',
        ' SUITE 4936  autres : code du caract\u00e8re'],
      ['Fichiers de donn\u00e9es (.TXT) :', ' OPEN "NOM" FOR INPUT AS #1', '  (ou OUTPUT, APPEND ; #1 ou #2)',
        ' PRINT #1,...  INPUT #1,v[,v...]', ' LINE INPUT #1,A$  EOF(1)', ' CLOSE [#1]', '',
        'Broches :', ' PINMODE b,m (0 entr\u00e9e, 1 sortie,', '  2 entr\u00e9e avec r\u00e9sistance de tirage',
        '  interne, bouton vers GND : appuy\u00e9=0)', ' DWRITE b,v  AWRITE b,v (0 \u00e0 255)', ' DREAD(b)  AREAD(a)', '']
    ];

    function helpPage(page) {
      newScreen();
      col = 0;
      attributs(INVERSION_FOND);
      puts(' reBASIC - Aide ');
      printNum(page + 1);
      puts('/');
      printNum(HELP_PAGES);
      puts(' ');
      attributs(FOND_NORMAL);
      newLine();
      newLine();
      for (const l of AIDE[page]) {
        if (l === '') newLine();
        else puts(l + '\n');
      }
      if (page === 3) {
        puts('Touches :\n ');
        keyName('ENVOI'); puts(' valide  ');
        keyName('CORRECTION'); puts(' efface\n ');
        keyName('ANNULATION'); puts(' interrompt\n ');
        keyName('GUIDE'); puts(' aide  ');
        keyName('SOMMAIRE'); puts(' menu\n');
      }
      moveXY(1, 24);
      col = 0;
      if (page > 0) keyName('RETOUR');
      if (page > 0 && page < HELP_PAGES - 1) puts(' - ');
      if (page < HELP_PAGES - 1) keyName('SUITE');
      puts(' - ');
      keyName('ENVOI');
      puts(' : reBASIC');
    }

    function* help() {
      let page = 0;
      helpPage(page);
      while (true) {
        const k = env.cle();
        if (k === 0) { yield ATTENTE_CLE; continue; }
        if (k === SUITE) {
          if (page < HELP_PAGES - 1) helpPage(++page);
          else bip();
        }
        else if (k === RETOUR) {
          if (page > 0) helpPage(--page);
          else bip();
        }
        else if (k === ENVOI || k === 0x0D) break;
      }
      newScreen();
      col = 0;
      gfxClear();
    }

    //////////////////////////////////////////////////////////////////////////////
    // Clavier

    function keyToStr(k) {
      if (k === 0x2191) k = 0x5E;  // La flèche vers le haut sert d'opérateur puissance
      if (k >= 0x20 && k < 0x7F) return String.fromCharCode(k);
      if (k >= 0x80 && k <= 0xFFFF) {
        const ch = String.fromCharCode(k);
        if (G2[ch]) return ch;
      }
      return '';
    }

    function erase() {
      const larg = (style & ST_LARGE) ? 2 : 1;
      if (col < larg) {
        moveUp(1);
        moveRight(40 - larg);
        col = 40 - larg;
      }
      else {
        moveLeft(larg);
        col -= larg;
      }
      clearLineFromCursor();
    }

    // Saisie d'une ligne. Renvoie [résultat, texte].
    function* readLine(initial, maxLen, garder, invite) {
      let buf = '';
      let resultat = -1;
      if (garder) { buf = initial; puts(buf); }
      wb(0x11);  // Curseur visible
      while (resultat < 0) {
        if (invite && env.commandeEnAttente()) { resultat = K_COMMANDE; break; }
        if (arretExterne && !invite) { resultat = K_ANNULER; break; }
        const k = env.cle();
        if (k === 0) { yield ATTENTE_CLE; continue; }
        if (k === ENVOI || k === 0x0D) resultat = K_OK;
        else if (k === SOMMAIRE) resultat = K_QUITTER;
        else if (k === ANNULATION) resultat = K_ANNULER;
        else if (k === GUIDE) resultat = K_AIDE;
        else if (k === CORRECTION) {
          if (buf.length > 0) {
            buf = buf.slice(0, -1);
            erase();
          }
        }
        else {
          const s = keyToStr(k);
          if (s.length > 0) {
            if (u8len(buf) + u8len(s) <= maxLen) {
              buf += s;
              puts(s);
            }
            else bip();  // La ligne est pleine
          }
        }
      }
      wb(0x14);  // Curseur caché
      return [resultat, buf];
    }

    function checkBreak() {
      if (arretExterne) { err = E_BREAK; return; }
      const k = env.cle();
      if (k === 0) return;
      if (k === SOMMAIRE || k === ANNULATION) err = E_BREAK;
      else key = k;
    }

    function* wait(duree) {
      const debut = env.millis();
      while (env.millis() - debut < duree && err === 0) {
        checkBreak();
        if (err) break;
        yield { t: 'temps', fin: debut + duree };
      }
    }

    //////////////////////////////////////////////////////////////////////////////
    // Dessin (points semi-graphiques)

    function gfxGet(x, y) {
      if (x < 0 || x >= GW || y < 0 || y >= GH) return false;
      return gfx[y * GW + x] === 1;
    }

    function gfxSet(x, y, allume) {
      if (x < 0 || x >= GW || y < 0 || y >= GH) return;
      gfx[y * GW + x] = allume ? 1 : 0;
    }

    function gfxPoints(cx, cy) {
      const x = cx * 2, y = cy * 3;
      let p = 0;
      if (gfxGet(x, y)) p |= 32;
      if (gfxGet(x + 1, y)) p |= 16;
      if (gfxGet(x, y + 1)) p |= 8;
      if (gfxGet(x + 1, y + 1)) p |= 4;
      if (gfxGet(x, y + 2)) p |= 2;
      if (gfxGet(x + 1, y + 2)) p |= 1;
      return p;
    }

    function graphic(p) {
      // Le caractère semi-graphique Videotex qui a ces 6 points
      wb(0x20 | ((p & 32) ? 1 : 0) | ((p & 16) ? 2 : 0) | ((p & 8) ? 4 : 0) |
        ((p & 4) ? 8 : 0) | ((p & 2) ? 16 : 0) | ((p & 1) ? 64 : 0));
    }

    function gfxCell(cx, cy) {
      if (cx < 0 || cx > 39 || cy < 0 || cy > 23) return;
      if (cx === 39 && cy === 23) return;
      moveXY(cx + 1, cy + 1);
      wb(0x0E);
      graphic(gfxPoints(cx, cy));
      wb(0x0F);
      restoreAttr();
      col = (cx + 1) % 40;
    }

    function gfxMark(cases, x, y, allume) {
      if (x < 0 || x >= GW || y < 0 || y >= GH) return;
      gfxSet(x, y, allume);
      cases[Math.floor(y / 3) * 40 + Math.floor(x / 2)] = 1;
    }

    function gfxFlush(cases) {
      let graphique = false;
      let suivante = -1;
      for (let k = 0; k < 960; k++) {
        if (!cases[k]) continue;
        const cx = k % 40, cy = Math.floor(k / 40);
        if (cx === 39 && cy === 23) continue;
        if (k !== suivante) moveXY(cx + 1, cy + 1);
        if (!graphique) { wb(0x0E); graphique = true; }
        graphic(gfxPoints(cx, cy));
        suivante = (cx === 39) ? -1 : k + 1;
        col = (cx + 1) % 40;
      }
      if (graphique) {
        wb(0x0F);
        restoreAttr();
      }
    }

    function gfxBox(x1, y1, x2, y2, allume, plein) {
      const cases = new Uint8Array(960);
      if (x1 > x2) { const t = x1; x1 = x2; x2 = t; }
      if (y1 > y2) { const t = y1; y1 = y2; y2 = t; }
      const xa = x1 < 0 ? 0 : x1, xb = x2 >= GW ? GW - 1 : x2;
      const ya = y1 < 0 ? 0 : y1, yb = y2 >= GH ? GH - 1 : y2;
      if (plein) {
        for (let y = ya; y <= yb; y++) for (let x = xa; x <= xb; x++) gfxMark(cases, x, y, allume);
      }
      else {
        for (let x = xa; x <= xb; x++) { gfxMark(cases, x, y1, allume); gfxMark(cases, x, y2, allume); }
        for (let y = ya; y <= yb; y++) { gfxMark(cases, x1, y, allume); gfxMark(cases, x2, y, allume); }
      }
      gfxFlush(cases);
    }

    function gfxLine(x1, y1, x2, y2, allume) {
      const cases = new Uint8Array(960);
      const dx = x2 > x1 ? x2 - x1 : x1 - x2;
      const dy = y2 > y1 ? y1 - y2 : y2 - y1;
      const sx = x1 < x2 ? 1 : -1, sy = y1 < y2 ? 1 : -1;
      let ecart = dx + dy;
      while (true) {
        gfxMark(cases, x1, y1, allume);
        if (x1 === x2 && y1 === y2) break;
        const e2 = 2 * ecart;
        if (e2 >= dy) { ecart += dy; x1 += sx; }
        if (e2 <= dx) { ecart += dx; y1 += sy; }
      }
      gfxFlush(cases);
    }

    function circleHeight(dx, r) {
      const rx = F(r + 0.5);
      const ry = F(F(r * 1.25) + 0.5);
      const f = F(1 - F(F(dx * dx) / F(rx * rx)));
      if (f <= 0) return 0;
      return Math.trunc(F(ry * F(Math.sqrt(f))));
    }

    function gfxCircle(xc, yc, r, allume, plein) {
      const cases = new Uint8Array(960);
      for (let dx = 0; dx <= r; dx++) {
        const haut = circleHeight(dx, r);
        let bas = 0;
        if (!plein && dx < r) {
          bas = circleHeight(dx + 1, r) + 1;
          if (bas > haut) bas = haut;
        }
        if (yc - haut < 0 && yc + haut >= GH && plein) bas = 0;
        for (let dy = bas; dy <= haut; dy++) {
          if (yc + dy >= 0 && yc - dy < GH) {
            gfxMark(cases, xc + dx, yc + dy, allume);
            gfxMark(cases, xc - dx, yc + dy, allume);
            gfxMark(cases, xc + dx, yc - dy, allume);
            gfxMark(cases, xc - dx, yc - dy, allume);
          }
        }
      }
      gfxFlush(cases);
    }

    function gfxClear() { gfx.fill(0); }

    //////////////////////////////////////////////////////////////////////////////
    // Analyse du texte

    const isL = (c) => c >= 65 && c <= 90;
    const isD = (c) => c >= 48 && c <= 57;
    const cc = (i) => T.charCodeAt(i);

    function skip() { while (T.charCodeAt(P) === 32) P++; }

    function kw(mot) {
      skip();
      if (!T.startsWith(mot, P)) return false;
      const n = mot.length;
      if (isL(mot.charCodeAt(n - 1)) && isL(T.charCodeAt(P + n))) return false;
      P += n;
      return true;
    }

    function expect(ch) {
      skip();
      if (T[P] === ch) P++;
      else if (err === 0) err = E_SYNTAXE;
    }

    function nextWord(t, p) {
      // Prochain mot (suite de lettres) en dehors des chaînes ; -1 à la fin ou sur REM / '
      while (p < t.length) {
        const c = t.charCodeAt(p);
        if (c === 34) {
          p++;
          while (p < t.length && t.charCodeAt(p) !== 34) p++;
          if (p < t.length) p++;
        }
        else if (c === 39) return -1;
        else if (isL(c)) {
          if (t.startsWith('REM', p)) return -1;
          return p;
        }
        else p++;
      }
      return -1;
    }

    function wordEnd(t, p) { while (isL(t.charCodeAt(p))) p++; return p; }

    function atElse() { skip(); return T.startsWith('ELSE', P); }

    function findElse(p) {
      let niveau = 0;
      while ((p = nextWord(T, p)) >= 0) {
        if (T.startsWith('IF', p)) niveau++;
        else if (T.startsWith('ELSE', p)) {
          if (niveau === 0) return p + 4;
          niveau--;
        }
        p = wordEnd(T, p);
      }
      return -1;
    }

    function skipStatement(t, p) {
      while (p < t.length && t[p] !== ':') {
        if (t[p] === '"') {
          p++;
          while (p < t.length && t[p] !== '"') p++;
          if (p < t.length) p++;
        }
        else p++;
      }
      return p;
    }

    function upper(s) {
      let g = false, out = '';
      for (const ch of s) {
        if (ch === '"') { g = !g; out += ch; }
        else if (!g && ch >= 'a' && ch <= 'z') out += ch.toUpperCase();
        else out += ch;
      }
      return out;
    }

    function parseNum(t, p) {
      // Lit un nombre (sans signe) en t[p]. Renvoie [valeur, position] ou null.
      let v = 0;
      let chiffre = false;
      while (isD(t.charCodeAt(p))) { v = F(F(v * 10) + (t.charCodeAt(p) - 48)); p++; chiffre = true; }
      if (t[p] === '.') {
        p++;
        let poids = F(0.1);
        while (isD(t.charCodeAt(p))) {
          v = F(v + F((t.charCodeAt(p) - 48) * poids));
          poids = F(poids / 10);
          p++;
          chiffre = true;
        }
      }
      if (!chiffre) return null;
      if (t[p] === 'E' && (isD(t.charCodeAt(p + 1)) || ((t[p + 1] === '+' || t[p + 1] === '-') && isD(t.charCodeAt(p + 2))))) {
        p++;
        let negatif = false;
        if (t[p] === '+') p++;
        else if (t[p] === '-') { negatif = true; p++; }
        let e = 0;
        while (isD(t.charCodeAt(p))) { if (e < 100) e = e * 10 + (t.charCodeAt(p) - 48); p++; }
        while (e-- > 0) v = negatif ? F(v / 10) : F(v * 10);
      }
      return [v, p];
    }

    function isStrStart() {
      skip();
      let q = P;
      if (T[q] === '"') return true;
      if (!isL(cc(q))) return false;
      while (isL(cc(q))) q++;
      return T[q] === '$';
    }

    function relOp() {
      skip();
      const c = T[P];
      if (c === '=') { P++; return 1; }
      if (c === '<') {
        P++;
        if (T[P] === '>') { P++; return 2; }
        if (T[P] === '=') { P++; return 5; }
        return 3;
      }
      if (c === '>') {
        P++;
        if (T[P] === '=') { P++; return 6; }
        return 4;
      }
      return 0;
    }

    function compare(op, d) {
      let vrai = false;
      switch (op) {
        case 1: vrai = (d === 0); break;
        case 2: vrai = (d !== 0); break;
        case 3: vrai = (d < 0); break;
        case 4: vrai = (d > 0); break;
        case 5: vrai = (d <= 0); break;
        case 6: vrai = (d >= 0); break;
      }
      return vrai ? -1 : 0;
    }

    //////////////////////////////////////////////////////////////////////////////
    // Variables et tableaux

    function resetStacks() {
      gs = []; fors = []; whs = [];
      dataLigne = -1; dataPos = 0; dataIn = false;
    }

    function clearArrays() {
      arrBase.fill(-1);
      arrDim.fill(0);
      arrUsed = 0;
    }

    function clearVars() {
      vars.fill(0);
      strs.fill('');
      clearArrays();
      resetStacks();
    }

    function dim(idx, n) {
      // Les tableaux sont rangés à la fin de la mémoire, le dernier réservé le plus bas
      if (arrBase[idx] >= 0 || n < 0) { err = E_VALEUR; return; }
      if (n + 1 > Math.floor(libre() / 4)) { err = E_MEMOIRE; return; }
      arrUsed += n + 1;
      arrBase[idx] = MEM_FLOATS - arrUsed;
      arrDim[idx] = n;
      for (let i = 0; i <= n; i++) arr[arrBase[idx] + i] = 0;
    }

    function arrRef(idx) {
      // Lit "(indice)" et renvoie la position de l'élément dans arr (-1 en cas d'erreur)
      expect('(');
      const i = Math.trunc(expr());
      expect(')');
      if (err) return -1;
      if (arrBase[idx] < 0) {
        dim(idx, 10);
        if (err) return -1;
      }
      if (i < 0 || i > arrDim[idx]) { err = E_INDICE; return -1; }
      return arrBase[idx] + i;
    }

    //////////////////////////////////////////////////////////////////////////////
    // Chaînes de caractères

    function ajoute(dst, src) {
      // Ajoute src à dst sans dépasser STR_MAX octets ni couper un caractère
      let n = u8len(dst);
      for (const ch of src) {
        const l = u8c(ch);
        if (n + l > STR_MAX) break;
        dst += ch;
        n += l;
      }
      return dst;
    }

    function enter() {
      if (depth >= DEPTH_MAX) {
        if (err === 0) err = E_IMBRIC;
        return false;
      }
      depth++;
      return true;
    }

    function strPart(genre) {
      expect('(');
      const src = strExpr();
      expect(',');
      const a = Math.trunc(expr());
      let b = STR_MAX;
      skip();
      if (genre === 'M' && T[P] === ',') {
        P++;
        b = Math.trunc(expr());
      }
      expect(')');
      if (err) return '';
      if (a < 0 || b < 0 || (genre === 'M' && a < 1)) { err = E_VALEUR; return ''; }
      const car = Array.from(src);
      let morceau;
      if (genre === 'L') morceau = car.slice(0, a);
      else if (genre === 'R') morceau = car.slice(a < car.length ? car.length - a : 0);
      else morceau = car.slice(a - 1, a - 1 + b);
      return ajoute('', morceau.join(''));
    }

    function strCase(majuscules) {
      expect('(');
      const src = strExpr();
      expect(')');
      if (err) return '';
      let out = '';
      for (const ch of src) {
        if (majuscules && ch >= 'a' && ch <= 'z') out += ch.toUpperCase();
        else if (!majuscules && ch >= 'A' && ch <= 'Z') out += ch.toLowerCase();
        else out += ch;
      }
      return ajoute('', out);
    }

    function hexStr() {
      const v = arg();
      if (err) return '';
      if (v < -2147483648.0 || v > 4294967295.0) { err = E_VALEUR; return ''; }
      const n = v < 0 ? (Math.trunc(v) | 0) >>> 0 : Math.trunc(v) >>> 0;
      return n.toString(16).toUpperCase();
    }

    const deux = (n) => String(n % 100).padStart(2, '0');

    function clockStr(date) {
      if (!env.horlogeOK) { err = E_HORLOGE; return ''; }
      const t = env.horloge();
      if (date) return deux(t.getDate()) + '/' + deux(t.getMonth() + 1) + '/' + deux(Math.floor(t.getFullYear() / 100)) + deux(t.getFullYear());
      return deux(t.getHours()) + ':' + deux(t.getMinutes()) + ':' + deux(t.getSeconds());
    }

    function strPrimary() {
      skip();
      if (T[P] === '"') {
        P++;
        const debut = P;
        while (P < T.length && T[P] !== '"') P++;
        const s = ajoute('', T.slice(debut, P));
        if (T[P] === '"') P++;
        return s;
      }
      if (isL(cc(P)) && T[P + 1] === '$') {
        const s = strs[cc(P) - 65];
        P += 2;
        return s;
      }
      if (kw('INKEY$')) {
        if (key === 0) key = env.cle();
        const k = key;
        key = 0;
        if (k === SOMMAIRE || k === ANNULATION) { err = E_BREAK; return ''; }
        if (k === ENVOI || k === 0x0D) return '\r';
        return keyToStr(k);
      }
      if (kw('DATE$')) return clockStr(true);
      if (kw('TIME$')) return clockStr(false);
      let genre = '';
      if (kw('CHR$')) genre = 'C';
      else if (kw('STR$')) genre = 'S';
      else if (kw('LEFT$')) genre = 'L';
      else if (kw('RIGHT$')) genre = 'R';
      else if (kw('MID$')) genre = 'M';
      else if (kw('HEX$')) genre = 'H';
      else if (kw('UPPER$')) genre = 'U';
      else if (kw('LOWER$')) genre = 'W';
      if (genre === '') { err = E_TYPE; return ''; }
      if (!enter()) return '';
      let s = '';
      if (genre === 'C') {
        expect('(');
        const n = Math.trunc(expr());
        expect(')');
        if (err === 0) {
          if (n < 1 || n > 127) err = E_VALEUR;
          else s = String.fromCharCode(n);
        }
      }
      else if (genre === 'S') {
        expect('(');
        const v = expr();
        expect(')');
        if (err === 0) s = ajoute('', numToStr(v));
      }
      else if (genre === 'H') s = hexStr();
      else if (genre === 'U' || genre === 'W') s = strCase(genre === 'U');
      else s = strPart(genre);
      depth--;
      return s;
    }

    function strExpr() {
      let dst = strPrimary();
      while (err === 0) {
        skip();
        if (T[P] !== '+') break;
        P++;
        dst = ajoute(dst, strPrimary());
      }
      return dst;
    }

    //////////////////////////////////////////////////////////////////////////////
    // Expressions numériques

    function arg() {
      expect('(');
      const v = expr();
      expect(')');
      return v;
    }

    function strCompare() {
      const a = strExpr();
      if (err) return 0;
      const op = relOp();
      if (op === 0) { err = E_TYPE; return 0; }
      const b = strExpr();
      if (err) return 0;
      return compare(op, a < b ? -1 : a > b ? 1 : 0);
    }

    function strToNum(s) {
      let q = 0;
      while (s[q] === ' ') q++;
      const negatif = (s[q] === '-');
      if (s[q] === '-' || s[q] === '+') q++;
      const r = parseNum(s, q);
      const v = r ? r[0] : 0;
      return negatif ? -v : v;
    }

    function strFunc(quoi) {
      expect('(');
      const s = strExpr();
      expect(')');
      if (err) return 0;
      if (quoi === 'N') return Array.from(s).length;
      if (quoi === 'C') return s.length ? versOctets(s)[0] : 0;
      return strToNum(s);
    }

    function instr() {
      let debut = 1;
      expect('(');
      if (!isStrStart()) {
        debut = Math.trunc(expr());
        expect(',');
      }
      const a = strExpr();
      expect(',');
      const b = strExpr();
      expect(')');
      if (err) return 0;
      if (debut < 1) { err = E_VALEUR; return 0; }
      const car = Array.from(a);
      if (debut > car.length + 1) return 0;
      const depart = car.slice(debut - 1).join('');
      const i = depart.indexOf(b);
      if (i < 0) return 0;
      return debut + Array.from(depart.slice(0, i)).length;
    }

    function clockNum(quoi) {
      if (!env.horlogeOK) { err = E_HORLOGE; return 0; }
      const t = env.horloge();
      switch (quoi) {
        case 'H': return t.getHours();
        case 'N': return t.getMinutes();
        case 'S': return t.getSeconds();
        case 'D': return t.getDate();
        case 'M': return t.getMonth() + 1;
      }
      return t.getFullYear();
    }

    function pinOK(b) {
      if (esp) {
        if (b < 0 || b > 39) return false;
        if (b >= 6 && b <= 11) return false;
        if (b === 16 || b === 17) return false;
        if (b === 21 || b === 22) return false;
        if (b === 5 || b === 18 || b === 19 || b === 23) return false;
      }
      else {
        if (b < 0 || b >= 70) return false;
        if (b <= 1) return false;
        if (b >= 16 && b <= 21) return false;
        if (b === 34 || b === 36) return false;
        if (b >= 50 && b <= 53) return false;
      }
      if (b === 2 || b === (esp ? 5 : 53)) return false;  // BUZZER, CHIP_SELECT
      return true;
    }

    function pinRead(analogique) {
      const n = Math.trunc(arg());
      if (err) return 0;
      if (esp) {
        if (!pinOK(n)) { err = E_BROCHE; return 0; }
      }
      else if (analogique) {
        if (n < 0 || n > 15) { err = E_BROCHE; return 0; }
      }
      else if (!pinOK(n)) { err = E_BROCHE; return 0; }
      return env.broches.lire(n, analogique);
    }

    function primary() {
      skip();
      const c = cc(P);

      if (isStrStart()) {
        if (!enter()) return 0;
        const v = strCompare();
        depth--;
        return v;
      }

      if (isD(c) || c === 46) {
        const r = parseNum(T, P);
        if (!r) { err = E_SYNTAXE; return 0; }
        P = r[1];
        return r[0];
      }

      if (c === 40) {
        if (!enter()) return 0;
        const v = arg();
        depth--;
        return v;
      }

      if (isL(c) && isL(cc(P + 1))) {
        if (!enter()) return 0;
        let v = 0;
        if (kw('LEN') || kw('VAL') || kw('ASC')) v = strFunc(T[P - 1]);
        else if (kw('ABS')) { v = arg(); if (v < 0) v = -v; }
        else if (kw('INT')) v = Math.floor(arg());
        else if (kw('SGN')) { v = arg(); v = v > 0 ? 1 : (v < 0 ? -1 : 0); }
        else if (kw('SQR')) { v = arg(); if (v < 0) err = E_VALEUR; else v = F(Math.sqrt(v)); }
        else if (kw('SIN')) v = F(Math.sin(arg()));
        else if (kw('COS')) v = F(Math.cos(arg()));
        else if (kw('TAN')) v = F(Math.tan(arg()));
        else if (kw('ATN')) v = F(Math.atan(arg()));
        else if (kw('LOG')) { v = arg(); if (v <= 0) err = E_VALEUR; else v = F(Math.log(v)); }
        else if (kw('EXP')) v = F(Math.exp(arg()));
        else if (kw('POINT')) {
          expect('(');
          const x = Math.trunc(expr());
          expect(',');
          const y = Math.trunc(expr());
          expect(')');
          if (err === 0 && x >= 0 && x < GW && y >= 0 && y < GH) v = gfxGet(x, y) ? -1 : 0;
        }
        else if (kw('INSTR')) v = instr();
        else if (kw('KEY')) {
          if (key === 0) key = env.cle();
          const k = key;
          key = 0;
          if (k === SOMMAIRE || k === ANNULATION) err = E_BREAK;
          else v = k;
        }
        else if (kw('TIMER')) v = Math.floor(env.millis() - timer0);
        else if (kw('HOUR')) v = clockNum('H');
        else if (kw('MINUTE')) v = clockNum('N');
        else if (kw('SECOND')) v = clockNum('S');
        else if (kw('DAY')) v = clockNum('D');
        else if (kw('MONTH')) v = clockNum('M');
        else if (kw('YEAR')) v = clockNum('Y');
        else if (kw('DREAD')) v = pinRead(false);
        else if (kw('AREAD')) v = pinRead(true);
        else if (kw('EOF')) {
          const n = Math.trunc(arg());
          if (err === 0) {
            if (n < 1 || n > FILE_MAX) err = E_VALEUR;
            else if (!fichiers[n - 1]) err = E_NONOUVERT;
            else v = fichiers[n - 1].disponible() ? 0 : -1;
          }
        }
        else if (kw('RND')) {
          v = arg();
          if (v <= 1) v = F(Math.floor(env.hasard() * 10000) / 10000);
          else v = 1 + Math.floor(env.hasard() * Math.trunc(v));
        }
        else err = E_SYNTAXE;
        depth--;
        return F(v);
      }

      if (isL(c)) {
        const idx = c - 65;
        P++;
        if (T[P] === '(') {
          if (!enter()) return 0;
          const i = arrRef(idx);
          depth--;
          return i >= 0 ? arr[i] : 0;
        }
        return vars[idx];
      }

      err = E_SYNTAXE;
      return 0;
    }

    function power() {
      let v = primary();
      skip();
      if (err === 0 && T[P] === '^') {
        P++;
        v = F(Math.pow(v, unary()));
      }
      return v;
    }

    function unary() {
      skip();
      if (T[P] === '-') { P++; return -unary(); }
      if (T[P] === '+') { P++; return unary(); }
      return power();
    }

    function mul() {
      let v = unary();
      while (err === 0) {
        skip();
        if (T[P] === '*') { P++; v = F(v * unary()); }
        else if (T[P] === '/') {
          P++;
          const d = unary();
          if (d === 0) { if (err === 0) err = E_DIVZERO; }
          else v = F(v / d);
        }
        else if (kw('MOD')) {
          const d = Math.trunc(unary());
          if (d === 0) { if (err === 0) err = E_DIVZERO; }
          else v = F(Math.trunc(v) % d);
        }
        else break;
      }
      return v;
    }

    function add() {
      let v = mul();
      while (err === 0) {
        skip();
        if (T[P] === '+') { P++; v = F(v + mul()); }
        else if (T[P] === '-') { P++; v = F(v - mul()); }
        else break;
      }
      return v;
    }

    function rel() {
      let v = add();
      if (err) return v;
      const op = relOp();
      if (op !== 0) v = compare(op, F(v - add()));
      return v;
    }

    function not() {
      if (kw('NOT')) return ~Math.trunc(not());
      return rel();
    }

    function and() {
      let v = not();
      while (err === 0 && kw('AND')) {
        const d = Math.trunc(not());
        v = Math.trunc(v) & d;
      }
      return v;
    }

    function expr() {
      let v = and();
      while (err === 0 && kw('OR')) {
        const d = Math.trunc(and());
        v = Math.trunc(v) | d;
      }
      return F(v);
    }

    //////////////////////////////////////////////////////////////////////////////
    // Le programme en mémoire

    function findLine(num) {
      let i = 0;
      while (i < prog.length && prog[i].n < num) i++;
      return i;
    }

    const tailleLigne = (t) => 3 + tailleTokenisee(t) + 1;

    function storeLine(num, texte) {
      const i = findLine(num);
      let ancienne = 0;
      if (i < prog.length && prog[i].n === num) ancienne = tailleLigne(prog[i].t);
      const t = texte.replace(/ +$/, '');
      const nouvelle = t.length > 0 ? tailleLigne(t) : 0;
      if (nouvelle - ancienne > libre()) {
        // Hors d'un programme en cours, les tableaux sont oubliés pour faire de la place
        // (jamais pendant un programme ou un CHAIN), comme basStoreLine
        if (arrUsed === 0 || cur >= 0 || chaine) return false;
        clearArrays();
        if (nouvelle - ancienne > libre()) return false;
      }
      dataLigne = -1;
      dataIn = false;
      if (ancienne > 0) { prog.splice(i, 1); progLen -= ancienne; }
      if (nouvelle > 0) { prog.splice(i, 0, { n: num, t: t }); progLen += nouvelle; }
      return true;
    }

    function enterLine(texte) {
      let p = 0, num = 0;
      while (isD(texte.charCodeAt(p))) {
        if (num < 100000) num = num * 10 + (texte.charCodeAt(p) - 48);
        p++;
      }
      if (num < 1 || num > 65535) { err = E_SYNTAXE; return; }
      while (texte[p] === ' ') p++;
      if (!storeLine(num, texte.slice(p))) err = E_MEMOIRE;
    }

    function goto_(num) {
      const i = findLine(num);
      if (i >= prog.length || prog[i].n !== num) { err = E_LIGNE; errLigne = num; return; }
      aller(i, 0);
      jumped = true;
    }

    function lineArg() {
      const v = expr();
      if (err) return 0;
      if (v < 1 || v > 65535) { err = E_LIGNE; return 0; }
      return Math.trunc(v);
    }

    //////////////////////////////////////////////////////////////////////////////
    // Fichiers sur la carte SD

    function fileName(extension, pcPermis) {
      // pcPermis : le préfixe "PC:" est accepté (SAVE, LOAD, MERGE). Sur la carte, le
      // fichier est alors sur l'ordinateur relié par le câble USB ; le Minitel virtuel n'a
      // pas de câble : « PC absent », comme une carte sans passerelle.
      let s = strExpr();
      if (err) return null;
      const pc = pcPermis && /^pc:/i.test(s);
      if (pc) s = s.slice(3);
      const n = s.length;
      let point = -1;
      for (let i = 0; i < n; i++) {
        if (s[i] === '.') {
          if (point >= 0) { err = E_VALEUR; return null; }
          point = i;
        }
      }
      const base = point < 0 ? n : point;
      const ext = point < 0 ? 0 : n - point - 1;
      if (base < 1 || base > 8 || (point >= 0 && (ext < 1 || ext > 3))) { err = E_VALEUR; return null; }
      let nom = '';
      for (let i = 0; i < n; i++) {
        let c = s[i];
        if (c >= 'a' && c <= 'z') c = c.toUpperCase();
        const k = c.charCodeAt(0);
        if (!isL(k) && !isD(k) && c !== '_' && c !== '-' && i !== point) { err = E_VALEUR; return null; }
        nom += c;
      }
      if (point < 0) nom += extension;
      if (pc) { err = E_PC; return null; }
      return nom;
    }

    function programmeTexte() {
      let s = '';
      for (const l of prog) s += l.n + ' ' + l.t + '\r\n';
      return s;
    }

    function save() {
      const nom = fileName('.BAS', true);
      if (!nom) return;
      if (!env.sd.ecrire(nom, versOctets(programmeTexte()))) err = E_FICHIER;
    }

    function chargerTexte(texte, fusion, enchainer) {
      // Les lignes d'un texte, comme LOAD (fusion faux) ou MERGE (fusion vrai) ; CHAIN
      // (enchainer vrai) garde les variables, les tableaux et les fichiers ouverts
      stop = true;
      if (enchainer) {
        prog = [];
        progLen = 0;
        resetStacks();
        chaine = true;
      }
      else if (!fusion) {
        closeAll();
        prog = [];
        progLen = 0;
        clearVars();
      }
      else resetStacks();
      for (let l of texte.split('\n')) {
        l = u8tronquer(l.replace(/\r/g, ''), LINE_MAX);
        l = upper(l).replace(/^ +/, '');
        if (isD(l.charCodeAt(0))) enterLine(l);
        if (err) break;
      }
    }

    function load(fusion, enchainer) {
      // LOAD, MERGE (fusion vrai) ou CHAIN (enchainer vrai : le programme est lancé)
      const nom = fileName('.BAS', true);
      if (!nom) return;
      if (!env.sd.existe(nom)) { err = E_INTROUV; return; }
      chargerTexte(depuisOctets(env.sd.lire(nom)), fusion, enchainer);
      chaine = false;
      if (!enchainer) return;
      if (err) { cur = -1; T = ''; P = -1; return; }  // L'ancien programme n'est plus là (pas de ligne à montrer)
      if (prog.length === 0) return;
      aller(0, 0);  // On lance le nouveau programme, à sa première ligne
      jumped = true;
      stop = false;
    }

    function* vdt() {
      // VDT "NOM" : envoie au Minitel la page Vidéotex NOM.VDT, comme basVdt
      const nom = fileName('.VDT', false);
      if (!nom) return;
      if (!env.sd.existe(nom)) { err = E_INTROUV; return; }
      const octets = env.sd.lire(nom);
      for (let i = 0; i < octets.length && err === 0; i++) {
        wb(octets[i] & 0x7F);
        if (i % 32 === 31) {
          checkBreak();
          while (env.fileOctets() > 64 && err === 0) yield ATTENTE_FLUX;
        }
      }
      wb(0x0F);  // Retour aux caractères texte
      wb(0x14);  // Curseur caché
      resetAttr();
      col = 0;
      gfxClear();
    }

    function files() {
      if (isStrStart()) {  // FILES "PC:" : pas de PC relié au Minitel virtuel
        const s = strExpr();
        if (err) return;
        err = /^pc:?$/i.test(s) ? E_PC : E_VALEUR;
        return;
      }
      for (const f of env.sd.liste()) {
        if (err) break;
        let nom = f.nom.slice(0, 13);
        if (nom.length > 4 && nom.slice(-4).toUpperCase() === '.BAS') nom = nom.slice(0, -4);
        puts(nom);
        while (col < 13) { wb(0x20); avance(); }
        printNum(f.taille);
        newLine();
        checkBreak();
      }
    }

    function closeAll() {
      for (let i = 0; i < FILE_MAX; i++) {
        if (fichiers[i]) { fichiers[i].fermer(); fichiers[i] = null; }
      }
    }

    function ouvrirFichier(nom, mode) {
      // Un fichier ouvert : en lecture (octets), en écriture (morceaux ajoutés)
      if (mode === 'I') {
        const octets = env.sd.lire(nom);
        let pos = 0;
        return {
          lire: () => (pos < octets.length ? octets[pos++] : -1),
          disponible: () => pos < octets.length,
          ecrire: () => { },
          fermer: () => { }
        };
      }
      let contenu = (mode === 'A' && env.sd.existe(nom)) ? Array.from(env.sd.lire(nom)) : [];
      if (mode === 'O') env.sd.ecrire(nom, new Uint8Array(0));
      let ouvert = true;
      return {
        lire: () => -1,
        disponible: () => false,
        ecrire: (s) => { for (const b of versOctets(s)) contenu.push(b); },
        fermer: () => { if (ouvert) env.sd.ecrire(nom, Uint8Array.from(contenu)); ouvert = false; }
      };
    }

    function channel(ouvert) {
      skip();
      if (T[P] === '#') P++;
      const n = Math.trunc(expr());
      if (err) return -1;
      if (n < 1 || n > FILE_MAX) { err = E_VALEUR; return -1; }
      if (ouvert && !fichiers[n - 1]) { err = E_NONOUVERT; return -1; }
      return n - 1;
    }

    function open() {
      const nom = fileName('.TXT');
      if (!nom) return;
      if (!kw('FOR')) { err = E_SYNTAXE; return; }
      let mode;
      if (kw('INPUT')) mode = 'I';
      else if (kw('OUTPUT')) mode = 'O';
      else if (kw('APPEND')) mode = 'A';
      else { err = E_SYNTAXE; return; }
      if (!kw('AS')) { err = E_SYNTAXE; return; }
      const i = channel(false);
      if (err) return;
      if (fichiers[i]) { err = E_DEJA; return; }
      if (mode === 'I' && !env.sd.existe(nom)) { err = E_INTROUV; return; }
      fichiers[i] = ouvrirFichier(nom, mode);
    }

    function close() {
      skip();
      if (P >= T.length || T[P] === ':' || atElse()) { closeAll(); return; }
      while (err === 0) {
        const i = channel(false);
        if (err) return;
        if (fichiers[i]) { fichiers[i].fermer(); fichiers[i] = null; }
        skip();
        if (T[P] !== ',') break;
        P++;
      }
    }

    function fileField(f, ligne) {
      // Lit une valeur (jusqu'à une virgule ou une fin de ligne) ou une ligne entière.
      // Renvoie null si le fichier est terminé.
      let c = f.lire();
      if (!ligne) while (c === 32) c = f.lire();
      if (c < 0) return null;
      const out = [];
      const guillemets = (!ligne && c === 34);
      let ferme = false;
      if (guillemets) c = f.lire();
      while (c >= 0 && c !== 10) {
        if (c === 13) { }
        else if (guillemets && !ferme) {
          if (c === 34) ferme = true;
          else if (out.length < STR_MAX) out.push(c);
        }
        else if (!ligne && c === 44) break;
        else if (!ferme && out.length < STR_MAX) out.push(c);
        c = f.lire();
      }
      if (!guillemets && !ligne) while (out.length > 0 && out[out.length - 1] === 32) out.pop();
      // Pas de caractère UTF-8 coupé à la fin
      let k = out.length;
      while (k > 0 && (out[k - 1] & 0xC0) === 0x80) k--;
      if (k > 0 && (out[k - 1] & 0x80) !== 0) {
        const c0 = out[k - 1];
        const attendu = (c0 & 0xE0) === 0xC0 ? 2 : (c0 & 0xF0) === 0xE0 ? 3 : 4;
        if (out.length - (k - 1) < attendu) out.length = k - 1;
      }
      return depuisOctets(Uint8Array.from(out));
    }

    function cible() {
      // Lit une variable à remplir : { genre: 'n' | 's' | 't', idx | i }
      skip();
      if (!isL(cc(P)) || isL(cc(P + 1))) { err = E_SYNTAXE; return null; }
      const idx = cc(P) - 65;
      P++;
      if (T[P] === '$') { P++; return { genre: 's', idx }; }
      if (T[P] === '(') {
        const i = arrRef(idx);
        if (i < 0) return null;
        return { genre: 't', i };
      }
      return { genre: 'n', idx };
    }

    function ranger(c, v) {
      if (c.genre === 's') strs[c.idx] = v;
      else if (c.genre === 't') arr[c.i] = v;
      else vars[c.idx] = v;
    }

    function fileInput(ligne) {
      skip();
      if (T[P] !== '#') { err = E_SYNTAXE; return; }
      const i = channel(true);
      expect(',');
      if (err) return;
      while (err === 0) {
        const c = cible();
        if (!c) return;
        if (ligne && c.genre !== 's') { err = E_TYPE; return; }
        const s = fileField(fichiers[i], ligne);
        if (s === null) { err = E_FINFICH; return; }
        ranger(c, c.genre === 's' ? s : strToNum(s));
        if (ligne) break;
        skip();
        if (T[P] === ',') P++;
        else break;
      }
    }

    //////////////////////////////////////////////////////////////////////////////
    // Instructions

    function listPages() {
      // L'option /P de LIST (avant ou après les numéros de lignes)
      skip();
      if (T[P] !== '/') return false;
      P++;
      skip();
      if (T[P] !== 'P') { err = E_SYNTAXE; return false; }
      P++;
      return true;
    }

    function* listPause() {
      // LIST /P : l'écran est plein. SUITE (ou ENVOI) continue ; ANNULATION ou SOMMAIRE arrête
      keyName('SUITE');
      puts(' : suite - ');
      keyName('ANNULATION');
      puts(' : arr\u00eat');
      let k = 0;
      while (k !== SUITE && k !== ENVOI && k !== 0x0D && k !== ANNULATION && k !== SOMMAIRE) {
        if (arretExterne) { k = ANNULATION; break; }
        k = env.cle();
        if (k === 0) yield ATTENTE_CLE;
      }
      wb(0x0D);
      clearLineFromCursor();
      col = 0;
      return k !== ANNULATION && k !== SOMMAIRE;
    }

    function* list() {
      // LIST [n][-m] [/P] : avec /P, la liste s'arrête à chaque écran plein
      let debut = 1, fin = 65535;
      let pages = listPages();
      if (err) return;
      skip();
      let r = parseNum(T, P);
      if (r) { P = r[1]; debut = Math.trunc(r[0]); fin = debut; }
      skip();
      if (T[P] === '-') {
        P++;
        fin = 65535;
        skip();
        r = parseNum(T, P);
        if (r) { P = r[1]; fin = Math.trunc(r[0]); }
      }
      if (!pages) pages = listPages();
      if (err) return;
      const suite = P;
      let rangees = 0;  // Rangées de l'écran déjà remplies par la liste
      for (let i = findLine(debut); i < prog.length && prog[i].n <= fin && err === 0; i++) {
        // Rangées de cette ligne (40 caractères pile : deux rangées, comme sur le Minitel)
        const hauteur = Math.floor((String(prog[i].n).length + 1 + [...prog[i].t].length) / 40) + 1;
        if (pages && rangees + hauteur > 22) {
          if (!(yield* listPause())) break;
          rangees = 0;
        }
        printNum(prog[i].n);
        puts(' ');
        puts(prog[i].t);
        newLine();
        rangees += hauteur;
        checkBreak();
        while (env.fileOctets() > 64 && err === 0) yield ATTENTE_FLUX;
      }
      P = suite;
    }

    function print() {
      let f = null;
      let colFichier = 0;
      skip();
      if (T[P] === '#') {
        const i = channel(true);
        if (err) return;
        f = fichiers[i];
        skip();
        if (T[P] === ',') P++;
        else if (P < T.length && T[P] !== ':' && !atElse()) { err = E_SYNTAXE; return; }
      }
      const out = (s) => {
        if (f === null) puts(s);
        else { f.ecrire(s); colFichier += Array.from(s).length; }
      };
      let retourLigne = true;
      while (err === 0) {
        skip();
        const c = T[P];
        if (P >= T.length || c === ':' || atElse()) break;
        if (c === ';') { P++; retourLigne = false; }
        else if (c === ',') {
          P++;
          retourLigne = false;
          if (f !== null) out(',');
          else {
            do { wb(0x20); avance(); } while (col % 10 !== 0);
          }
        }
        else if (kw('TAB')) {
          const n = Math.trunc(arg());
          retourLigne = true;
          while (err === 0 && (f ? colFichier : col) < n - 1 && (f !== null || col < 39)) out(' ');
        }
        else if (isStrStart()) {
          const s = strExpr();
          if (err === 0) out(s);
          retourLigne = true;
        }
        else {
          const v = expr();
          if (err === 0) out(numToStr(v));
          retourLigne = true;
        }
      }
      if (err === 0 && retourLigne) {
        if (f === null) newLine();
        else f.ecrire('\r\n');
      }
    }

    function* input() {
      skip();
      if (T[P] === '#') { fileInput(false); return; }
      if (T[P] === '"') {
        puts(strPrimary());
        skip();
        if (T[P] === ';' || T[P] === ',') P++;
        else { err = E_SYNTAXE; return; }
      }
      while (err === 0) {
        const c = cible();
        if (!c) return;
        let correct = false;
        while (!correct) {
          puts('? ');
          key = 0;
          const [r, texte] = yield* readLine('', STR_MAX, false, false);
          newLine();
          if (r === K_QUITTER || r === K_ANNULER) { err = E_BREAK; return; }
          if (c.genre === 's') {
            ranger(c, texte);
            correct = true;
          }
          else {
            let q = 0;
            while (texte[q] === ' ') q++;
            const negatif = (texte[q] === '-');
            if (texte[q] === '-' || texte[q] === '+') q++;
            const n = parseNum(upper(texte), q);
            if (n) {
              ranger(c, negatif ? -n[0] : n[0]);
              correct = true;
            }
            else puts('Nombre attendu\n');
          }
        }
        skip();
        if (T[P] === ',') P++;
        else break;
      }
    }

    function for_() {
      skip();
      if (!isL(cc(P)) || isL(cc(P + 1))) { err = E_SYNTAXE; return; }
      const idx = cc(P) - 65;
      P++;
      expect('=');
      const debut = expr();
      if (err) return;
      if (!kw('TO')) { err = E_SYNTAXE; return; }
      const limite = expr();
      let pas = 1;
      if (err === 0 && kw('STEP')) pas = expr();
      if (err) return;
      for (let i = 0; i < fors.length; i++) {
        if (fors[i].v === idx) { fors.length = i; break; }
      }
      if (fors.length >= FOR_MAX) { err = E_IMBRIC; return; }
      vars[idx] = debut;
      fors.push({ v: idx, lim: limite, pas: pas, l: cur, p: P });
    }

    function next() {
      skip();
      if (isL(cc(P)) && !isL(cc(P + 1))) {
        const idx = cc(P) - 65;
        P++;
        while (fors.length > 0 && fors[fors.length - 1].v !== idx) fors.pop();
      }
      if (fors.length === 0) { err = E_NEXT; return; }
      const b = fors[fors.length - 1];
      const v = F(vars[b.v] + b.pas);
      vars[b.v] = v;
      if ((b.pas >= 0 && v <= b.lim) || (b.pas < 0 && v >= b.lim)) aller(b.l, b.p);
      else fors.pop();
    }

    function let_() {
      skip();
      if (!isL(cc(P)) || isL(cc(P + 1))) { err = E_SYNTAXE; return; }
      const idx = cc(P) - 65;
      P++;
      if (T[P] === '$') {
        P++;
        expect('=');
        if (err) return;
        const s = strExpr();
        if (err === 0) strs[idx] = s;
      }
      else if (T[P] === '(') {
        const i = arrRef(idx);
        expect('=');
        if (err) return;
        const v = expr();
        if (err === 0) arr[i] = v;
      }
      else {
        expect('=');
        if (err) return;
        const v = expr();
        if (err === 0) vars[idx] = v;
      }
    }

    function gosub(num) {
      if (gs.length >= GOSUB_MAX) { err = E_IMBRIC; return; }
      gs.push({ l: cur, p: P });
      goto_(num);
      if (err) gs.pop();
    }

    function* if_() {
      const v = expr();
      if (err) return;
      if (!kw('THEN')) {
        skip();
        if (!T.startsWith('GOTO', P)) { err = E_SYNTAXE; return; }
      }
      if (v === 0) {
        const suite = findElse(P);
        if (suite < 0) { P = T.length; return; }
        P = suite;
      }
      skip();
      if (isD(cc(P))) {
        const num = lineArg();
        if (err === 0) goto_(num);
      }
      else yield* statement();
    }

    function on() {
      const x = Math.trunc(expr());
      if (err) return;
      let sousProgramme;
      if (kw('GOTO')) sousProgramme = false;
      else if (kw('GOSUB')) sousProgramme = true;
      else { err = E_SYNTAXE; return; }
      let cibleNum = 0, rang = 1;
      while (true) {
        const num = lineArg();
        if (err) return;
        if (rang === x) cibleNum = num;
        skip();
        if (T[P] !== ',') break;
        P++;
        rang++;
      }
      if (cibleNum === 0) return;
      if (sousProgramme) gosub(cibleNum);
      else goto_(cibleNum);
    }

    function findWend(l, p) {
      let niveau = 0;
      let t = texteDe(l);
      while (true) {
        p = nextWord(t, p);
        while (p < 0) {
          if (l < 0) return null;
          l++;
          if (l >= prog.length) return null;
          t = prog[l].t;
          p = nextWord(t, 0);
        }
        if (t.startsWith('WHILE', p)) niveau++;
        else if (t.startsWith('WEND', p)) {
          if (niveau === 0) return { l, p: p + 4 };
          niveau--;
        }
        p = wordEnd(t, p);
      }
    }

    function while_(debut) {
      const v = expr();
      if (err) return;
      if (v !== 0) {
        for (let i = 0; i < whs.length; i++) {
          if (whs[i].l === cur && whs[i].p === debut) { whs.length = i; break; }
        }
        if (whs.length >= WHILE_MAX) { err = E_IMBRIC; return; }
        whs.push({ l: cur, p: debut });
      }
      else {
        const r = findWend(cur, P);
        if (!r) { err = E_WHILE; return; }
        aller(r.l, r.p);
        jumped = true;
      }
    }

    function findData(t, p) {
      while (p < t.length) {
        while (t[p] === ' ') p++;
        if (t.startsWith('DATA', p)) return p + 4;
        if (t.startsWith('REM', p) || t[p] === "'") return -1;
        p = skipStatement(t, p);
        if (t[p] === ':') p++;
      }
      return -1;
    }

    function dataNext() {
      if (dataLigne < 0) { dataLigne = 0; dataPos = 0; dataIn = false; }
      while (!dataIn) {
        if (dataLigne >= prog.length) { err = E_DATA; return null; }
        const q = findData(prog[dataLigne].t, dataPos);
        if (q >= 0) { dataPos = q; dataIn = true; }
        else { dataLigne++; dataPos = 0; }
      }
      const t = prog[dataLigne].t;
      let p = dataPos;
      while (t[p] === ' ') p++;
      let out;
      if (t[p] === '"') {
        p++;
        const debut = p;
        while (p < t.length && t[p] !== '"') p++;
        out = ajoute('', t.slice(debut, p));
        if (t[p] === '"') p++;
        while (t[p] === ' ') p++;
      }
      else {
        const debut = p;
        while (p < t.length && t[p] !== ',' && t[p] !== ':') p++;
        let fin = p;
        while (fin > debut && t[fin - 1] === ' ') fin--;
        out = ajoute('', t.slice(debut, fin));
      }
      if (t[p] === ',') p++;
      else {
        dataIn = false;
        if (t[p] === ':') p++;
      }
      dataPos = p;
      return out;
    }

    function isNumber(s) {
      let q = 0;
      while (s[q] === ' ') q++;
      if (s[q] === '-' || s[q] === '+') q++;
      const r = parseNum(s, q);
      if (!r) return false;
      q = r[1];
      while (s[q] === ' ') q++;
      return q >= s.length;
    }

    function read() {
      while (err === 0) {
        const c = cible();
        if (!c) return;
        const donnee = dataNext();
        if (donnee === null) return;
        if (c.genre === 's') ranger(c, donnee);
        else if (isNumber(donnee)) ranger(c, strToNum(donnee));
        else { err = E_TYPE; return; }
        skip();
        if (T[P] === ',') P++;
        else break;
      }
    }

    function restore() {
      skip();
      dataIn = false;
      if (P >= T.length || T[P] === ':' || atElse()) { dataLigne = -1; return; }
      const num = lineArg();
      if (err) return;
      const i = findLine(num);
      if (i >= prog.length || prog[i].n !== num) { err = E_LIGNE; errLigne = num; return; }
      dataLigne = i;
      dataPos = 0;
    }

    function pinWrite(quoi) {
      const broche = Math.trunc(expr());
      expect(',');
      const v = Math.trunc(expr());
      if (err) return;
      if (!pinOK(broche)) { err = E_BROCHE; return; }
      if (quoi === 'M') {
        if (v >= 0 && v <= 2) env.broches.mode(broche, v);
        else err = E_VALEUR;
      }
      else if (quoi === 'D') env.broches.ecrire(broche, v !== 0 ? 1 : 0);
      else {
        if (v < 0 || v > 255) { err = E_VALEUR; return; }
        env.broches.ecrireAnalogique(broche, v);
      }
    }

    function renumMap(ancien, debut, pas) {
      for (let rang = 0; rang < prog.length; rang++) {
        const num = prog[rang].n;
        if (num === ancien) return debut + rang * pas;
        if (num > ancien) break;
      }
      return -1;
    }

    function renumText(src, debut, pas) {
      // Le texte d'une ligne avec les numéros renouvelés après GOTO, GOSUB, THEN, ELSE et
      // RESTORE ; null s'il devient trop long
      let out = '';
      let p = 0;
      while (p < src.length) {
        const c = src[p];
        if (c === '"') {
          const q = src.indexOf('"', p + 1);
          const fin = q < 0 ? src.length : q + 1;
          out += src.slice(p, fin);
          p = fin;
          continue;
        }
        if (c === "'" || src.startsWith('REM', p)) { out += src.slice(p); break; }
        if (!isL(src.charCodeAt(p))) { out += c; p++; continue; }
        let lg = 0, liste = false;
        if (src.startsWith('GOTO', p)) { lg = 4; liste = true; }
        else if (src.startsWith('GOSUB', p)) { lg = 5; liste = true; }
        else if (src.startsWith('THEN', p) || src.startsWith('ELSE', p)) lg = 4;
        else if (src.startsWith('RESTORE', p)) lg = 7;
        if (lg === 0) {
          while (isL(src.charCodeAt(p))) out += src[p++];
          continue;
        }
        out += src.slice(p, p + lg);
        p += lg;
        while (true) {
          while (src[p] === ' ') out += src[p++];
          if (!isD(src.charCodeAt(p))) break;
          const debutNum = p;
          let ancien = 0;
          while (isD(src.charCodeAt(p))) { if (ancien < 100000) ancien = ancien * 10 + (src.charCodeAt(p) - 48); p++; }
          const nouveau = renumMap(ancien, debut, pas);
          out += nouveau >= 0 ? String(nouveau) : src.slice(debutNum, p);
          if (!liste) break;
          let q = p;
          while (src[q] === ' ') q++;
          if (src[q] !== ',') break;
          out += src.slice(p, q + 1);
          p = q + 1;
        }
      }
      return u8len(out) > TEXT_MAX ? null : out;
    }

    function renum() {
      let debut = 10, pas = 10;
      skip();
      if (P < T.length && T[P] !== ':' && T[P] !== ',') debut = Math.trunc(expr());
      skip();
      if (err === 0 && T[P] === ',') {
        P++;
        pas = Math.trunc(expr());
      }
      if (err) return;
      if (debut < 1 || debut > 65535 || pas < 1 || pas > 65535) { err = E_VALEUR; return; }
      if (prog.length === 0) return;
      if (debut + (prog.length - 1) * pas > 65535) { err = E_VALEUR; return; }
      const textes = [];
      let total = progLen, maxi = progLen;
      for (const l of prog) {
        const t = renumText(l.t, debut, pas);
        if (t === null) { err = E_MEMOIRE; return; }
        total += tailleTokenisee(t) - tailleTokenisee(l.t);
        if (total > maxi) maxi = total;
        textes.push(t);
      }
      if (maxi > MEM_SIZE - 4 * arrUsed) { err = E_MEMOIRE; return; }
      prog = prog.map((l, i) => ({ n: debut + i * pas, t: textes[i] }));
      progLen = total;
      resetStacks();
      stop = true;
    }

    function* statement() {
      skip();
      const debut = P;
      if (T.startsWith('REM', P) || T[P] === "'") P = T.length;
      else if (kw('ELSE')) P = T.length;
      else if (T[P] === '?') { P++; print(); }
      else if (kw('PRINT')) print();
      else if (kw('INPUT')) yield* input();
      else if (kw('IF')) yield* if_();
      else if (kw('GOTO')) {
        const num = lineArg();
        if (err === 0) goto_(num);
      }
      else if (kw('GOSUB')) {
        const num = lineArg();
        if (err === 0) gosub(num);
      }
      else if (kw('ON')) on();
      else if (kw('RETURN')) {
        if (gs.length === 0) { err = E_RETURN; return; }
        const r = gs.pop();
        aller(r.l, r.p);
      }
      else if (kw('FOR')) for_();
      else if (kw('NEXT')) next();
      else if (kw('WHILE')) while_(debut);
      else if (kw('WEND')) {
        if (whs.length === 0) { err = E_WEND; return; }
        const w = whs.pop();
        aller(w.l, w.p);
        jumped = true;
      }
      else if (kw('DATA')) P = skipStatement(T, P);
      else if (kw('READ')) read();
      else if (kw('RESTORE')) restore();
      else if (kw('END')) { closeAll(); stop = true; }
      else if (kw('STOP')) stop = true;
      else if (kw('DIM')) {
        let encore = true;
        while (encore && err === 0) {
          skip();
          if (!isL(cc(P)) || T[P + 1] !== '(') { err = E_SYNTAXE; return; }
          const idx = cc(P) - 65;
          P++;
          const n = Math.trunc(arg());
          if (err === 0) dim(idx, n);
          skip();
          encore = (T[P] === ',');
          if (encore) P++;
        }
      }
      else if (kw('CLS')) {
        newScreen();
        col = 0;
        gfxClear();
        ink = 7; paper = 0; style = 0;
      }
      else if (kw('PLOT') || kw('LINE') || kw('BOX') || kw('CIRCLE')) {
        const genre = T[debut];
        if (genre === 'L' && kw('INPUT')) { fileInput(true); return; }
        const c = [0, 0, 0, 0];
        const n = genre === 'P' ? 2 : genre === 'C' ? 3 : 4;
        for (let i = 0; i < n && err === 0; i++) {
          if (i > 0) expect(',');
          const v = expr();
          if (v < -1000 || v > 1000) { if (err === 0) err = E_VALEUR; }
          c[i] = Math.trunc(v);
        }
        let allume = true, plein = false;
        skip();
        if (err === 0 && T[P] === ',') {
          P++;
          allume = (expr() !== 0);
          skip();
          if (err === 0 && T[P] === ',' && (genre === 'B' || genre === 'C')) {
            P++;
            plein = (expr() !== 0);
          }
        }
        if (err) return;
        if (genre === 'C' && c[2] < 0) { err = E_VALEUR; return; }
        if (genre === 'P') {
          if (c[0] >= 0 && c[0] < GW && c[1] >= 0 && c[1] < GH) {
            gfxSet(c[0], c[1], allume);
            gfxCell(Math.floor(c[0] / 2), Math.floor(c[1] / 3));
          }
        }
        else if (genre === 'L') gfxLine(c[0], c[1], c[2], c[3], allume);
        else if (genre === 'B') gfxBox(c[0], c[1], c[2], c[3], allume, plein);
        else gfxCircle(c[0], c[1], c[2], allume, plein);
      }
      else if (kw('LOCATE')) {
        const x = Math.trunc(expr());
        expect(',');
        const y = Math.trunc(expr());
        if (err) return;
        if (x < 1 || x > 40 || y < 1 || y > 24) { err = E_VALEUR; return; }
        moveXY(x, y);
        col = x - 1;
      }
      else if (kw('COLOR') || kw('PAPER') || kw('SIZE')) {
        const genre = T[debut];
        const n = Math.trunc(expr());
        if (err) return;
        if (n < 0 || n > (genre === 'S' ? 3 : 7)) { err = E_VALEUR; return; }
        if (genre === 'C') { attributs(CARACTERE_NOIR + n); ink = n; }
        else if (genre === 'P') { attributs(FOND_NOIR + n); paper = n; }
        else {
          const hautAvant = (style & ST_HAUT) !== 0;
          const haut = (n & 1) !== 0;
          if (haut && !hautAvant) attributs(GRANDEUR_NORMALE + n);
          else esc(GRANDEUR_NORMALE + n);
          style = (style & 0x0F) | (n << 4);
        }
        attr = true;
      }
      else if (kw('INVERSE') || kw('FLASH') || kw('UNDERLINE')) {
        const genre = T[debut];
        const n = Math.trunc(expr());
        if (err) return;
        const bit = genre === 'I' ? ST_INVERSE : genre === 'F' ? ST_FLASH : ST_SOULIGNE;
        if (genre === 'I') attributs(n !== 0 ? INVERSION_FOND : FOND_NORMAL);
        else if (genre === 'F') attributs(n !== 0 ? CLIGNOTEMENT : FIXE);
        else attributs(n !== 0 ? DEBUT_LIGNAGE : FIN_LIGNAGE);
        if (n !== 0) style |= bit;
        else style &= ~bit;
        attr = true;
      }
      else if (kw('EDIT')) {
        const num = lineArg();
        if (err) return;
        const i = findLine(num);
        if (i >= prog.length || prog[i].n !== num) { err = E_LIGNE; errLigne = num; return; }
        const s = num + ' ' + prog[i].t;
        if (u8len(s) > LINE_MAX) { err = E_LONGUE; return; }
        inBuf = s;
        editPending = true;
        stop = true;
      }
      else if (kw('BEEP')) bip();
      else if (kw('SOUND')) {
        const frequence = Math.trunc(expr());
        expect(',');
        const duree = Math.trunc(expr());
        if (err) return;
        if (frequence < 31 || frequence > 20000 || duree < 0 || duree > 60000) { err = E_VALEUR; return; }
        env.son(frequence, duree);
        yield* wait(duree);
        env.sonStop();
      }
      else if (kw('PAUSE')) {
        const duree = Math.trunc(expr());
        if (err) return;
        if (duree < 0) { err = E_VALEUR; return; }
        yield* wait(duree);
      }
      else if (kw('RUN')) {
        closeAll();
        clearVars();
        timer0 = env.millis();
        if (prog.length === 0) { stop = true; return; }
        aller(0, 0);
        jumped = true;
      }
      else if (kw('LIST')) yield* list();
      else if (kw('NEW')) {
        closeAll();
        prog = [];
        progLen = 0;
        clearVars();
        stop = true;
      }
      else if (kw('MEM')) {
        printNum(libre());
        puts(' octets libres\n');
      }
      else if (kw('AIDE') || kw('HELP')) yield* help();
      else if (kw('SAVE')) save();
      else if (kw('LOAD')) load(false);
      else if (kw('MERGE')) load(true);
      else if (kw('CHAIN')) load(false, true);
      else if (kw('VDT')) yield* vdt();
      else if (kw('RENUM')) renum();
      else if (kw('OPEN')) open();
      else if (kw('CLOSE')) close();
      else if (kw('PINMODE')) pinWrite('M');
      else if (kw('DWRITE')) pinWrite('D');
      else if (kw('AWRITE')) pinWrite('A');
      else if (kw('FILES')) files();
      else if (kw('KILL')) {
        const nom = fileName('.BAS');
        if (!nom) return;
        if (!env.sd.existe(nom)) err = E_INTROUV;
        else if (!env.sd.supprimer(nom)) err = E_FICHIER;
      }
      else {
        kw('LET');
        let_();
      }
    }

    function* rythme() {
      // L'Arduino attend que la liaison série ait envoyé ses données (tampon de 64 octets),
      // et il n'exécute qu'un certain nombre d'instructions par seconde
      while (env.fileOctets() > 64 && err === 0) yield ATTENTE_FLUX;
      const cadence = env.cadence();
      const t = env.millis();
      if (cadence > 0) {
        rythmeT = Math.max(rythmeT, t - 50) + 1000 / cadence;
        if (rythmeT - t > 12) yield { t: 'temps', fin: rythmeT };
      }
      else if (t - trancheT > 12) {
        trancheT = t;
        yield TRANCHE;
      }
    }

    function* run() {
      enCours = true;
      rythmeT = env.millis();
      while (err === 0 && !stop) {
        checkBreak();
        if (err) break;
        skip();
        if (T[P] === ':') P++;
        else if (P >= T.length) {
          if (cur < 0) break;
          if (cur + 1 >= prog.length) { closeAll(); break; }
          aller(cur + 1, 0);
        }
        else {
          jumped = false;
          depth = 0;
          yield* statement();
          if (err || stop) break;
          if (!jumped) {
            skip();
            if (P < T.length && T[P] !== ':' && !atElse()) err = E_SYNTAXE;
          }
          yield* rythme();
        }
      }
      enCours = false;
    }

    function* executerLigne(texte) {
      // Une ligne validée par ENVOI (ou envoyée par la page)
      ligneDirecte = upper(texte);
      let p = 0;
      while (ligneDirecte[p] === ' ') p++;
      err = 0;
      aller(-1, p);
      if (isD(cc(p))) {
        enterLine(ligneDirecte.slice(p));
        if (err) printError();
      }
      else if (p < ligneDirecte.length) {
        stop = false;
        attr = false;
        key = 0;
        gs = []; fors = []; whs = [];
        yield* run();
        if (err && !(err === E_BREAK && arretSilencieux)) printError();
        arretExterne = false;
        arretSilencieux = false;
        if (cur >= 0) closeAll();
        if (attr) resetAttr();
        if (col !== 0) newLine();
        if (!editPending && env.afficherOk) puts('Ok\n');
      }
    }

    function* basic(accueil, lancer) {
      newScreen();
      col = 0;
      gfxClear();
      if (prog.length === 0) clearVars();
      timer0 = env.millis();
      if (accueil) {
        puts('reBASIC - ');
        printNum(libre());
        puts(' octets libres\n');
        keyName('GUIDE');
        puts(' : aide - ');
        keyName('SOMMAIRE');
        puts(' : retour au menu');
        if (col !== 0) newLine();  // 40 caractères pile : le Minitel est déjà passé à la ligne
        if (env.afficherOk) puts('Ok\n');
      }
      if (lancer) {
        // Le gestionnaire de la carte SD (menu 3) a demandé de lancer un programme : LOAD
        // puis RUN, affichés comme si on les avait tapés (comme basic() dans Basic.ino)
        const charger = 'LOAD "' + lancer + '"';
        puts(charger); newLine();
        yield* executerLigne(charger);
        if (err === 0) { puts('RUN'); newLine(); yield* executerLigne('RUN'); }
      }
      while (true) {
        const edition = editPending;
        editPending = false;
        const [r, texte] = yield* readLine(edition ? inBuf : '', LINE_MAX, edition, true);
        if (r === K_QUITTER) break;
        else if (r === K_ANNULER) newLine();
        else if (r === K_AIDE) {
          yield* help();
          if (env.afficherOk) puts('Ok\n');
        }
        else if (r === K_COMMANDE) {
          // Commande envoyée par la page (lancer, executer)
          const cmd = env.commandeEnAttente();
          env.commandeFaite();
          if (texte.length > 0 || col !== 0) newLine();
          if (cmd.echo) { puts(cmd.texte); newLine(); }
          yield* executerLigne(cmd.texte);
        }
        else {
          newLine();
          yield* executerLigne(texte);
        }
      }
      closeAll();
      wb(0x14);
    }

    //////////////////////////////////////////////////////////////////////////////
    // rebootMinitel : l'écran d'accueil et ses menus (Menus.ino, Horloge.ino, APropos.ino)

    function println(s) { puts(s); wb(0x0D); wb(0x0A); col = 0; }

    function* pause(ms) {
      // delay() de l'Arduino : rien d'autre ne se passe
      const fin = env.millis() + ms;
      while (env.millis() < fin) yield { t: 'pause', fin };
    }

    function* bipOK() {
      for (let i = 0; i < 3; i++) {
        env.son(262, 125);       // NOTE_C4, une croche
        yield* pause(162);
        env.sonStop();
      }
    }

    function* bipErreur() {
      env.son(65, 1000);         // NOTE_C2
      yield* pause(1300);
      env.sonStop();
    }

    function* attendreCle() {
      while (true) {
        if (env.commandeEnAttente()) return 0;
        const k = env.cle();
        if (k) return k;
        yield ATTENTE_CLE;
      }
    }

    function afficherTouche(nom) {
      attributs(INVERSION_FOND);
      puts(nom);
      attributs(FOND_NORMAL);
    }

    const date2 = (n) => String(n).padStart(2, '0');
    function texteDateHeure(t) {
      return date2(t.getDate()) + '/' + date2(t.getMonth() + 1) + '/' + String(t.getFullYear()).padStart(4, '0') +
        '  ' + date2(t.getHours()) + ':' + date2(t.getMinutes()) + ':' + date2(t.getSeconds());
    }

    function* ecranAccueil() {
      // Renvoie la touche choisie (0x30 à 0x35), ou 0x31 si la page envoie une commande
      // (lancer un programme : on va tout de suite dans reBASIC, sans dessiner l'accueil)
      if (env.commandeEnAttente()) return 0x31;
      let seconde = -1;
      newScreen();
      wb(0x14);
      // entropie (police slscript)
      moveRight(15); println('_/_');
      moveRight(6); println('_  ____  /  __  __ _   o _');
      moveRight(5); println('</_/ / <_<__/ (_(_)/_)_<_</_');
      moveRight(23); println('/');
      moveRight(22); println("'");
      // LAB (police graffiti)
      moveRight(5); println('.____       _____ __________');
      moveRight(5); println('|    |     /  _  \\\\______   \\');
      moveRight(5); println('|    |    /  /_\\  \\|    |  _/');
      moveRight(5); println('|    |___/    |    \\    |   \\');
      moveRight(5); println('|_______ \\____|__  /______  /');
      moveRight(13); println('\\/       \\/       \\/');
      // reboot
      for (let i = 0; i < 2; i++) wb(0x0A);
      println('rebootMinitel');
      println('V.' + env.versionReboot);
      // MENU
      moveXY(20, 14);
      attributs(INVERSION_FOND);
      puts('MENU');
      attributs(FOND_NORMAL);
      moveXY(20, 16); println('0 - R\u00e9glage Horloge');
      moveXY(20, 17); println('1 - reBASIC');
      moveXY(20, 18); println('2 - Pages Vid\u00e9otex');
      moveXY(20, 19); println('3 - Fichiers');
      moveXY(20, 20); println('4 - Pong');
      moveXY(20, 21);
      println('5 - \u00c0 propos');  // Le Minitel affiche « A propos » : pas de majuscule accentuée
      while (true) {
        if (env.commandeEnAttente()) return 0x31;
        const k = env.cle();
        if (k >= 0x30 && k <= 0x35) return k;
        // L'horloge, en bas de l'écran, mise à jour à chaque seconde
        if (env.horlogeOK) {
          const t = env.horloge();
          if (t.getSeconds() !== seconde) {
            seconde = t.getSeconds();
            wb(0x14);
            moveXY(11, 24);
            puts(texteDateHeure(t));
          }
        }
        yield { t: 'temps', fin: env.millis() + 100 };
      }
    }

    function* reglageHorloge() {
      const debut = 12;
      moveXY(debut, 11); puts('JJ/MM/AAAA  hh:mm');
      moveXY(debut, 12); puts('  /  /        :  ');
      moveXY(1, 23); afficherTouche('CORRECTION'); puts(' : effacer');
      moveXY(1, 24); afficherTouche('SOMMAIRE'); puts(' : retour au menu sans modifier');
      const position = [0, 1, 3, 4, 6, 7, 8, 9, 12, 13, 15, 16];
      let chaine = '';
      let abandon = false;
      moveXY(debut + position[0], 12);
      wb(0x11);
      while (chaine.length < 12 && !abandon) {
        const k = yield* attendreCle();
        if (k === 0) { abandon = true; break; }
        if (k >= 0x30 && k <= 0x39) {
          wb(k);
          chaine += String.fromCharCode(k);
          if (chaine.length < 12) moveXY(debut + position[chaine.length], 12);
        }
        else if (k === CORRECTION && chaine.length > 0) {
          chaine = chaine.slice(0, -1);
          moveXY(debut + position[chaine.length], 12);
          wb(0x20);
          moveXY(debut + position[chaine.length], 12);
        }
        else if (k === SOMMAIRE) abandon = true;
      }
      wb(0x14);
      if (abandon) return;
      const n = (i) => parseInt(chaine.substr(i, 2), 10);
      const jours = n(0), mois = n(2), annees = n(4) * 100 + n(6), heures = n(8), minutes = n(10);
      const joursMois = (mois === 4 || mois === 6 || mois === 9 || mois === 11) ? 30 : mois === 2 ? (annees % 4 === 0 ? 29 : 28) : 31;
      const valide = annees >= 2000 && annees <= 2099 && mois >= 1 && mois <= 12 && jours >= 1 && jours <= joursMois &&
        heures <= 23 && minutes <= 59;
      if (valide) {
        env.reglerHorloge(new Date(annees, mois - 1, jours, heures, minutes, 0));
        yield* bipOK();
      }
      else yield* bipErreur();
    }

    function* aPropos() {
      wb(0x14);
      const x = Math.floor((40 - 25) / 2) + 1;  // Centré sur « rebootMinitel (2023-2026) »
      moveXY(x, 7); puts('rebootMinitel (2023-2026)');
      moveXY(x, 9); puts("Un projet d'Eric S\u00e9randour");
      moveXY(x, 11); puts('bas\u00e9 sur sa biblioth\u00e8que');
      moveXY(x, 13); puts('Minitel1B_Hard (2016-2026)');
      moveXY(x, 15); puts('Logiciel Libre (GNU GPLv3)');
      moveXY(x, 17); puts('https://entropie.org/3615');
      moveXY(1, 24); afficherTouche('SOMMAIRE'); puts(' : retour au menu');
      while (true) {
        const k = yield* attendreCle();
        if (k === 0 || k === SOMMAIRE) return;
      }
    }

    function vdtPresentation(n) {
      // L'écran qui précède les pages : leur nombre et les touches (comme Videotex.ino)
      newScreen();
      wb(0x14);
      moveXY(1, 3); afficherTouche(' Pages Vid\u00e9otex ');
      if (n === 0) {
        moveXY(1, 7); puts('Aucune page sur la carte SD.');
        moveXY(1, 9); puts('Copiez des fichiers .VDT (pages');
        moveXY(1, 10); puts('Minitel) \u00e0 la racine de la carte.');
        moveXY(1, 24); afficherTouche('SOMMAIRE'); puts(' : retour');
        return;
      }
      moveXY(1, 7); puts(n + (n === 1 ? ' page sur la carte SD' : ' pages sur la carte SD'));
      moveXY(1, 10); afficherTouche('SUITE'); puts(' : page suivante');
      moveXY(1, 12); afficherTouche('RETOUR'); puts(' : page pr\u00e9c\u00e9dente');
      moveXY(1, 14); afficherTouche('ENVOI'); puts(' : d\u00e9filement automatique');
      moveXY(1, 16); afficherTouche('SOMMAIRE'); puts(' : retour au menu');
    }

    function* vdtAttendre(defilement, debut) {
      // SUITE, RETOUR, ENVOI ou SOMMAIRE ; en défilement automatique, SUITE au bout de 10 s
      while (true) {
        if (env.commandeEnAttente()) return SOMMAIRE;
        const k = env.cle();
        if (k === SOMMAIRE || k === SUITE || k === RETOUR || k === ENVOI) return k;
        if (defilement && env.millis() - debut >= VDT_DUREE) return SUITE;
        yield { t: 'temps', fin: env.millis() + 100 };
      }
    }

    function* vdtAfficher(nom) {
      // Envoie la page au Minitel ; renvoie la touche de fonction frappée pendant l'envoi, ou 0
      newScreen();
      const octets = env.sd.lire(nom);
      for (let i = 0; i < octets.length; i++) {
        wb(octets[i] & 0x7F);
        if (i % 32 === 31) {
          const k = env.cle();
          if (k === SOMMAIRE || k === SUITE || k === RETOUR || k === ENVOI) { wb(0x14); return k; }
          while (env.fileOctets() > 64) yield ATTENTE_FLUX;
        }
      }
      wb(0x14);
      return 0;
    }

    function* pagesVideotex() {
      // Menu 2 : les fichiers .VDT de la carte SD, par ordre alphabétique (comme Videotex.ino)
      const pages = env.sd.liste().map((f) => f.nom).filter((n) => /^.+\.VDT$/i.test(n)).sort();
      vdtPresentation(pages.length);
      if (pages.length === 0) {
        while ((yield* attendreCle()) !== SOMMAIRE && !env.commandeEnAttente());
        return;
      }
      // SUITE ou ENVOI (défilement automatique) : la première page ; ensuite, rien que les pages
      let k = 0;
      while (k !== SUITE && k !== ENVOI && k !== SOMMAIRE) k = yield* vdtAttendre(false, 0);
      if (k === SOMMAIRE) return;
      let defilement = (k === ENVOI);
      if (defilement) yield* bipOK();
      let i = -1, sens = 1, changer = true, debut = 0;
      while (true) {
        k = 0;
        if (changer) {
          i = (i + sens + pages.length) % pages.length;
          k = yield* vdtAfficher(pages[i]);
          debut = env.millis();
        }
        if (k === 0) k = yield* vdtAttendre(defilement, debut);
        if (k === SOMMAIRE) break;
        if (k === ENVOI) {
          defilement = !defilement;
          if (defilement) yield* bipOK(); else bip();
          debut = env.millis();
          changer = false;
        }
        else { sens = (k === RETOUR) ? -1 : 1; changer = true; }
      }
      newScreen();
      wb(0x0F);
      wb(0x14);
    }

    const FICH_PAR_PAGE = 16;

    function fichTitre(titre) {
      newScreen(); wb(0x14);
      moveXY(1, 1); afficherTouche(titre);
    }

    function fichTaille(taille, rangee) {
      const t = String(taille);
      moveXY(41 - t.length, rangee); puts(t);
    }

    function fichListe(fichiers, page) {
      // Un écran de la liste (comme fichListe dans Fichiers.ino)
      const nombre = fichiers.length, pages = Math.ceil(nombre / FICH_PAR_PAGE);
      fichTitre(' Carte SD ');
      moveXY(12, 1); puts(nombre + (nombre === 1 ? ' fichier' : ' fichiers'));
      if (pages > 1) { const t = (page + 1) + '/' + pages; moveXY(41 - t.length, 1); puts(t); }
      if (nombre === 0) {
        moveXY(1, 4); puts('La carte SD est vide, absente ou');
        moveXY(1, 5); puts('illisible.');
      }
      fichiers.slice(page * FICH_PAR_PAGE, (page + 1) * FICH_PAR_PAGE).forEach((f, i) => {
        moveXY(1, 3 + i); afficherTouche(String.fromCharCode(0x41 + i)); puts(' ');
        puts(f.nom.length > 26 ? f.nom.slice(0, 25) + '~' : f.nom);
        fichTaille(f.taille, 3 + i);
      });
      moveXY(1, 21); puts("Tapez la lettre d'un fichier.");
      moveXY(1, 22); afficherTouche('ENVOI'); puts(' : fichiers du PC');
      if (pages > 1) { moveXY(1, 23); afficherTouche('RETOUR'); puts(' - '); afficherTouche('SUITE'); puts(' : autres fichiers'); }
      moveXY(1, 24); afficherTouche('SOMMAIRE'); puts(' : retour au menu');
    }

    // Copie avec le PC (comme Fichiers.ino) : sur la carte, elle passe par la passerelle et
    // le câble USB. Le Minitel virtuel n'est relié à aucune carte : « PC absent », comme une
    // carte sans passerelle (les boutons « Ouvrir… » et « Enregistrer sous… » font ce travail).
    const NOM_83 = /^[A-Za-z0-9_-]{1,8}(\.[A-Za-z0-9_-]{1,3})?$/;

    function* fichVersPC(nom) {
      moveXY(1, 17);
      if (!NOM_83.test(nom)) { puts('Nom trop long pour le PC (format 8.3)'); yield* bipErreur(); return; }
      puts('Copie vers le PC...');
      moveXY(1, 19); puts('PC absent : la passerelle tourne ?'); clearLineFromCursor();
      yield* bipErreur();
    }

    function* pcFichiers() {
      fichTitre(' PC ');
      moveXY(1, 24); afficherTouche('SOMMAIRE'); puts(' : retour \u00e0 la carte SD');
      moveXY(1, 4); puts('PC absent : la passerelle tourne ?'); clearLineFromCursor();
      yield* bipErreur();
      yield* fichCle([SOMMAIRE]);
    }

    function* fichCle(acceptees) {
      // Attend l'une des touches données (ou une commande de la page : SOMMAIRE)
      while (true) {
        const k = yield* attendreCle();
        if (k === 0) return SOMMAIRE;
        if (acceptees.includes(k)) return k;
      }
    }

    function* fichAfficherTexte(nom) {
      // Le fichier comme du texte, 22 rangées par écran (comme fichAfficherTexte)
      const lignes = [];
      for (const brute of depuisOctets(env.sd.lire(nom)).replace(/\r/g, '').split('\n')) {
        let l = [...brute.replace(/\t/g, ' ')].filter((c) => c.codePointAt(0) >= 0x20);
        do { lignes.push(l.slice(0, 40).join('')); l = l.slice(40); } while (l.length > 0);
      }
      if (lignes.length && lignes[lignes.length - 1] === '') lignes.pop();
      newScreen();
      let rangee = 1;
      for (let i = 0; i < lignes.length; i++) {
        moveXY(1, rangee); puts(lignes[i]);
        while (env.fileOctets() > 64) yield ATTENTE_FLUX;
        if (++rangee > 22 && i < lignes.length - 1) {
          moveXY(1, 24); afficherTouche('SUITE'); puts(' : suite - '); afficherTouche('SOMMAIRE'); puts(' : retour');
          if ((yield* fichCle([SUITE, ENVOI, SOMMAIRE])) === SOMMAIRE) return;
          newScreen();
          rangee = 1;
        }
      }
      moveXY(1, 24); afficherTouche('SOMMAIRE'); puts(' : retour (fin du fichier)');
      yield* fichCle([SOMMAIRE]);
    }

    function* fichFichier(f) {
      // L'écran d'un fichier : 0 retour à la liste, 1 supprimé, 2 programme lancé
      const programme = /\.BAS$/i.test(f.nom), page = /\.VDT$/i.test(f.nom);
      while (true) {
        fichTitre(' Carte SD ');
        moveXY(1, 4); puts(f.nom); fichTaille(f.taille, 4);
        moveXY(1, 5); puts(programme ? 'Programme reBASIC' : page ? 'Page Vid\u00e9otex' : 'Fichier');
        moveXY(1, 9); puts('1 - Afficher');
        if (programme) { moveXY(1, 11); puts('2 - Lancer'); }
        moveXY(1, 13); puts('3 - Supprimer');
        moveXY(1, 15); puts('4 - Copier sur le PC');
        moveXY(1, 24); afficherTouche('SOMMAIRE'); puts(' : retour \u00e0 la liste');
        let k;
        while (true) {
          k = yield* fichCle(programme ? [0x31, 0x32, 0x33, 0x34, SOMMAIRE] : [0x31, 0x33, 0x34, SOMMAIRE]);
          if (k !== 0x34) break;
          // On reste sur cet écran : le résultat de la copie s'affiche en dessous
          moveXY(1, 17); clearLineFromCursor();
          moveXY(1, 19); clearLineFromCursor();
          yield* fichVersPC(f.nom);
        }
        if (k === SOMMAIRE) return 0;
        if (k === 0x31) {
          if (page) { yield* vdtAfficher(f.nom); yield* fichCle([SOMMAIRE]); newScreen(); wb(0x0F); }
          else yield* fichAfficherTexte(f.nom);
        }
        else if (k === 0x32) {
          yield* basic(true, f.nom);
          if (env.surSommaire) env.surSommaire();
          return 2;
        }
        else {
          moveXY(1, 17); puts('Supprimer ce fichier ?'); clearLineFromCursor();
          moveXY(1, 19); afficherTouche('ENVOI'); puts(' : oui - '); afficherTouche('SOMMAIRE'); puts(' : non'); clearLineFromCursor();
          if ((yield* fichCle([ENVOI, SOMMAIRE])) === ENVOI) {
            env.sd.supprimer(f.nom);
            yield* bipOK();
            return 1;
          }
        }
      }
    }

    function* carteSD() {
      // Menu 3 (« Fichiers ») : le gestionnaire de la carte SD (comme Fichiers.ino)
      let page = 0;
      while (true) {
        const fichiers = env.sd.liste();
        const pages = Math.ceil(fichiers.length / FICH_PAR_PAGE);
        if (page >= pages) page = Math.max(0, pages - 1);
        fichListe(fichiers, page);
        let choix = -1;
        while (choix < 0) {
          let k = yield* attendreCle();
          if (k === 0 || k === SOMMAIRE) return;
          if (k === ENVOI) { yield* pcFichiers(); break; }
          if (k === SUITE || k === RETOUR) {
            const nouvelle = page + (k === SUITE ? 1 : -1);
            if (nouvelle >= 0 && nouvelle < pages) { page = nouvelle; break; }
            bip();
          }
          if (k >= 0x61 && k <= 0x7A) k -= 32;
          if (k >= 0x41 && k < 0x41 + FICH_PAR_PAGE) {
            const i = page * FICH_PAR_PAGE + (k - 0x41);
            if (i < fichiers.length) choix = i;
            else bip();
          }
        }
        if (choix < 0) continue;
        if ((yield* fichFichier(fichiers[choix])) === 2) return;
      }
    }

    // Pong (menu 4), comme Pong.ino : positions de la balle en 1/16 de point
    const PONG_HAUT = 3, PONG_BAS = 68, PONG_X_GAUCHE = 3, PONG_X_DROITE = 76, PONG_RAQUETTE = 9,
      PONG_PAS = 3, PONG_GAGNANT = 5, PONG_IMAGE = 60;

    function* pongPartie(mode) {
      // Une partie. mode : 0 Minitel contre Minitel (sans fin), 1 un joueur, 2 deux joueurs.
      // Renvoie 0 si l'on a quitté (SOMMAIRE), 1 ou 2 pour le gagnant (gauche, droite).
      const deuxJoueurs = (mode === 2);
      let bx = 0, by = 0, vx = 0, vy = 0, g = 0, d = 0, scoreG = 0, scoreD = 0;
      const fixe = (x, y) => (x === 39 && y > PONG_HAUT && y < PONG_BAS && (y - PONG_HAUT - 2) % 4 === 0) ||
        (x === PONG_X_GAUCHE && y >= g && y < g + PONG_RAQUETTE) || (x === PONG_X_DROITE && y >= d && y < d + PONG_RAQUETTE);
      const balle = (cases, allume) => {
        const x = bx >> 4, y = by >> 4;
        for (let dy = 0; dy < 2; dy++) for (let dx = 0; dx < 2; dx++) {
          if (allume || !fixe(x + dx, y + dy)) gfxMark(cases, x + dx, y + dy, allume);
        }
      };
      const raquette = (cases, x, haut, allume) => { for (let y = haut; y < haut + PONG_RAQUETTE; y++) gfxMark(cases, x, y, allume); };
      const bouger = (haut, dy) => Math.min(PONG_BAS - PONG_RAQUETTE, Math.max(PONG_HAUT + 1, haut + dy));
      const automate = (haut, x) => {
        // Une raquette jouée par le Minitel (comme pongAutomate)
        const vient = (x === PONG_X_DROITE) ? (vx > 0 && bx > (30 << 4)) : (vx < 0 && bx < (49 << 4));
        if (!vient) return haut;
        const cible = (by >> 4) - Math.trunc(PONG_RAQUETTE / 2);
        if (cible >= haut - 1 && cible <= haut + 1) return haut;
        return bouger(haut, cible < haut ? -1 : 1);
      };
      const score = () => {
        // Symétrique par rapport au milieu : unités de gauche en 15-16, premier chiffre de
        // droite en 25-26 (comme pongScore)
        const tg = String(scoreG), td = String(scoreD);
        moveXY(17 - 2 * tg.length, 1);
        wb(0x1B); wb(0x4E);  // Double largeur
        puts(tg);
        moveXY(25, 1);
        wb(0x1B); wb(0x4E);  // Un positionnement remet la taille normale
        puts(td);
        wb(0x1B); wb(GRANDEUR_NORMALE);
        col = 0;
      };
      const service = (vers) => {
        bx = 39 << 4; by = Math.trunc((PONG_HAUT + PONG_BAS) / 2) << 4;
        vx = vers < 0 ? -16 : 16;
        vy = Math.floor(env.hasard() * 25) - 12;
      };
      newScreen(); wb(0x14);
      gfxClear();
      let cases = new Uint8Array(960);
      for (let x = 0; x < 80; x++) { gfxMark(cases, x, PONG_HAUT, true); gfxMark(cases, x, PONG_BAS, true); }
      for (let y = PONG_HAUT + 2; y < PONG_BAS; y += 4) gfxMark(cases, 39, y, true);  // Le filet
      g = d = Math.trunc((PONG_HAUT + PONG_BAS - PONG_RAQUETTE) / 2);
      raquette(cases, PONG_X_GAUCHE, g, true);
      raquette(cases, PONG_X_DROITE, d, true);
      gfxFlush(cases);
      score();
      service(env.hasard() < 0.5 ? 1 : -1);
      let image = env.millis();
      let attente = env.millis() + 1000;
      while (true) {
        if (env.commandeEnAttente()) return 0;
        let k = env.cle();
        if (k === SOMMAIRE) return 0;
        if (k >= 0x61 && k <= 0x7A) k -= 32;
        if (((k === 0x41 || k === 0x51) && mode !== 0) || ((k === 0x50 || k === 0x4D) && deuxJoueurs)) {
          cases = new Uint8Array(960);
          const gauche = (k === 0x41 || k === 0x51);
          const x = gauche ? PONG_X_GAUCHE : PONG_X_DROITE;
          const dy = (k === 0x41 || k === 0x50) ? -PONG_PAS : PONG_PAS;
          if (gauche) { raquette(cases, x, g, false); g = bouger(g, dy); raquette(cases, x, g, true); }
          else { raquette(cases, x, d, false); d = bouger(d, dy); raquette(cases, x, d, true); }
          gfxFlush(cases);
        }
        if (env.millis() - image < PONG_IMAGE || env.millis() < attente) {
          yield { t: 'temps', fin: env.millis() + 10 };
          continue;
        }
        image = env.millis();
        cases = new Uint8Array(960);
        if (!deuxJoueurs) {
          const haut = automate(d, PONG_X_DROITE);
          if (haut !== d) { raquette(cases, PONG_X_DROITE, d, false); d = haut; raquette(cases, PONG_X_DROITE, d, true); }
        }
        if (mode === 0) {
          const haut = automate(g, PONG_X_GAUCHE);
          if (haut !== g) { raquette(cases, PONG_X_GAUCHE, g, false); g = haut; raquette(cases, PONG_X_GAUCHE, g, true); }
        }
        balle(cases, false);
        bx += vx; by += vy;
        let x = bx >> 4, y = by >> 4;
        if (y <= PONG_HAUT) { by = (PONG_HAUT + 1) << 4; vy = -vy; env.son(440, 20); }
        if (y + 1 >= PONG_BAS) { by = (PONG_BAS - 2) << 4; vy = -vy; env.son(440, 20); }
        y = by >> 4;
        if (vx < 0 && x <= PONG_X_GAUCHE + 1 && x >= PONG_X_GAUCHE - 1 && y + 1 >= g && y <= g + PONG_RAQUETTE - 1) {
          bx = (PONG_X_GAUCHE + 1) << 4; vx = -vx + 1;
          vy = (y - (g + Math.trunc(PONG_RAQUETTE / 2))) * 5;
          env.son(880, 20);
        }
        if (vx > 0 && x + 1 >= PONG_X_DROITE - 1 && x + 1 <= PONG_X_DROITE + 1 && y + 1 >= d && y <= d + PONG_RAQUETTE - 1) {
          bx = (PONG_X_DROITE - 2) << 4; vx = -vx - 1;
          vy = (y - (d + Math.trunc(PONG_RAQUETTE / 2))) * 5;
          env.son(880, 20);
        }
        vx = Math.max(-40, Math.min(40, vx));
        x = bx >> 4;
        if (x < 0 || x > 78) {
          if (x < 0) scoreD++; else scoreG++;
          env.son(220, 150);
          gfxFlush(cases);
          score();
          if (mode !== 0 && scoreG === PONG_GAGNANT) return 1;
          if (mode !== 0 && scoreD === PONG_GAGNANT) return 2;
          service(x < 0 ? 1 : -1);
          attente = env.millis() + 1000;
          continue;
        }
        balle(cases, true);
        gfxFlush(cases);
        while (env.fileOctets() > 64) yield ATTENTE_FLUX;
      }
    }

    function* pong() {
      // Menu 4 : le choix du nombre de joueurs, puis les parties (comme pong() dans Pong.ino)
      while (true) {
        newScreen(); wb(0x14);
        moveXY(1, 3); afficherTouche(' Pong ');
        moveXY(1, 6); puts('0 - Minitel contre Minitel');
        moveXY(1, 8); puts('1 - Un joueur (contre le Minitel)');
        moveXY(1, 10); puts('2 - Deux joueurs');
        moveXY(1, 14); puts('Joueur de gauche : A monte, Q descend');
        moveXY(1, 16); puts('Joueur de droite : P monte, M descend');
        moveXY(1, 19); puts('Le premier \u00e0 5 points gagne');
        moveXY(1, 20); puts('(Minitel contre Minitel : sans fin).');
        moveXY(1, 24); afficherTouche('SOMMAIRE'); puts(' : retour au menu');
        let k = 0;
        while (k !== 0x30 && k !== 0x31 && k !== 0x32 && k !== SOMMAIRE) { k = yield* attendreCle(); if (k === 0) return; }
        if (k === SOMMAIRE) break;
        const mode = k - 0x30;
        const gagnant = yield* pongPartie(mode);
        if (gagnant === 0) break;
        // Fin de la partie : le gagnant et ENVOI, centrés sur la rangée du bas
        const message = mode === 1 ? (gagnant === 1 ? 'Vous avez gagn\u00e9 !' : 'Le Minitel a gagn\u00e9 !') :
          (gagnant === 1 ? 'La gauche a gagn\u00e9 !' : 'La droite a gagn\u00e9 !');
        // Centrage exact : un nombre pair de caractères (deux ou trois espaces avant ENVOI)
        const espaces = (message.length % 2 === 1) ? '  ' : '   ';
        moveXY((40 - (message.length + espaces.length + 15)) / 2 + 1, 24);
        puts(message + espaces); afficherTouche('ENVOI'); puts(' : rejouer');
        yield* bipOK();
        k = 0;
        while (k !== ENVOI && k !== SOMMAIRE) { k = yield* attendreCle(); if (k === 0) return; }
        if (k === SOMMAIRE) break;
      }
      gfxClear();
      newScreen();
    }

    function* systeme() {
      // La boucle de rebootMinitel : accueil, puis le menu choisi, puis retour à l'accueil.
      // Au démarrage, si la carte SD contient AUTORUN.BAS, reBASIC le charge et le lance
      // aussitôt (comme setup() dans rebootMinitel.ino).
      if (env.sd.existe('AUTORUN.BAS')) {
        newScreen();
        yield* basic(true, 'AUTORUN');
        if (env.surSommaire) env.surSommaire();
        newScreen();
      }
      while (true) {
        const choix = yield* ecranAccueil();
        newScreen();
        if (choix === 0x30) yield* reglageHorloge();
        else if (choix === 0x31) {
          yield* basic(env.accueilBasic);
          if (env.surSommaire) env.surSommaire();
        }
        else if (choix === 0x32) yield* pagesVideotex();
        else if (choix === 0x33) yield* carteSD();
        else if (choix === 0x34) yield* pong();
        else if (choix === 0x35) yield* aPropos();
        newScreen();
      }
    }

    return {
      basic,
      systeme,
      charger(texte, fusion) {
        err = 0;
        if (enCours) { arretExterne = true; arretSilencieux = true; }
        const fin = enCours;
        chargerTexte(texte, fusion);
        const e = err;
        if (!fin) stop = false;
        err = fin ? E_BREAK : 0;
        return e ? MESSAGES[e] : null;
      },
      interrompre(silencieux) {
        if (enCours) { arretExterne = true; arretSilencieux = !!silencieux; }
      },
      programme: programmeTexte,
      enCours: () => enCours,
      octetsLibres: () => libre(),
      variables() {
        const o = {};
        for (let i = 0; i < 26; i++) {
          const l = String.fromCharCode(65 + i);
          if (vars[i] !== 0) o[l] = vars[i];
          if (strs[i] !== '') o[l + '$'] = strs[i];
        }
        return o;
      },
      _numToStr: numToStr
    };
  }

  ////////////////////////////////////////////////////////////////////////////////////
  // La carte SD (dans le navigateur : localStorage, ou en mémoire)
  ////////////////////////////////////////////////////////////////////////////////////

  function creerCarteSD(mode, prefixe) {
    const memoire = {};
    let stockage = null;
    if (mode !== 'memoire') {
      try {
        stockage = window.localStorage;
        const t = prefixe + '__test';
        stockage.setItem(t, '1');
        stockage.removeItem(t);
      }
      catch (e) { stockage = null; }
    }
    const lireBrut = (nom) => (stockage ? stockage.getItem(prefixe + nom) : (nom in memoire ? memoire[nom] : null));
    const carte = {
      existe: (nom) => lireBrut(nom) !== null,
      lire: (nom) => versOctets(lireBrut(nom) || ''),
      ecrire(nom, octets) {
        const s = depuisOctets(octets);
        try {
          if (stockage) stockage.setItem(prefixe + nom, s);
          else memoire[nom] = s;
          return true;
        }
        catch (e) { return false; }
      },
      supprimer(nom) {
        if (stockage) stockage.removeItem(prefixe + nom);
        else delete memoire[nom];
        return true;
      },
      liste() {
        let noms = [];
        if (stockage) {
          for (let i = 0; i < stockage.length; i++) {
            const k = stockage.key(i);
            if (k.startsWith(prefixe)) noms.push(k.slice(prefixe.length));
          }
        }
        else noms = Object.keys(memoire);
        noms.sort();
        return noms.map((nom) => ({ nom, taille: versOctets(lireBrut(nom)).length }));
      }
    };
    // Pour la page : du texte plutôt que des octets
    carte.lireTexte = (nom) => lireBrut(nom.toUpperCase());
    carte.ecrireTexte = (nom, texte) => carte.ecrire(nom.toUpperCase(), versOctets(texte.replace(/\r?\n/g, '\r\n')));
    return carte;
  }

  ////////////////////////////////////////////////////////////////////////////////////
  // Le son : buzzer de la carte (SOUND) et bip du Minitel (BEEP)
  ////////////////////////////////////////////////////////////////////////////////////

  function creerSon(volume) {
    let ctx = null, osc = null, gain = null;
    function contexte() {
      if (ctx) return ctx;
      const AC = typeof window !== 'undefined' && (window.AudioContext || window.webkitAudioContext);
      if (!AC) return null;
      ctx = new AC();
      return ctx;
    }
    function arreter() {
      if (osc) { try { osc.stop(); } catch (e) { } osc.disconnect(); osc = null; }
    }
    function jouer(frequence, duree, forme) {
      const c = contexte();
      if (!c || volume <= 0) return;
      if (c.state === 'suspended') c.resume();
      arreter();
      osc = c.createOscillator();
      gain = c.createGain();
      osc.type = forme;
      osc.frequency.value = frequence;
      gain.gain.value = volume;
      osc.connect(gain).connect(c.destination);
      osc.start();
      if (duree > 0) osc.stop(c.currentTime + duree / 1000);
    }
    return {
      debloquer() { const c = contexte(); if (c && c.state === 'suspended') c.resume(); },
      son: (f, d) => jouer(f, d, 'square'),
      bip: () => jouer(1300, 120, 'square'),
      arreter
    };
  }

  ////////////////////////////////////////////////////////////////////////////////////
  // Les broches (simulées ; la page peut les relier à ses propres éléments)
  ////////////////////////////////////////////////////////////////////////////////////

  function creerBroches(surBroche) {
    const modes = {}, valeurs = {};
    const debut = maintenant();
    return {
      mode(b, m) { modes[b] = m; if (surBroche) surBroche('mode', b, m); },
      ecrire(b, v) { valeurs[b] = v; if (surBroche) surBroche('numerique', b, v); },
      ecrireAnalogique(b, v) { valeurs[b] = v; if (surBroche) surBroche('analogique', b, v); },
      lire(b, analogique) {
        if (surBroche) {
          const v = surBroche(analogique ? 'lireAnalogique' : 'lire', b);
          if (typeof v === 'number') return v;
        }
        if (analogique) return Math.round(512 + 511 * Math.sin((maintenant() - debut) / 2000));
        if (b in valeurs && modes[b] === 1) return valeurs[b];
        return modes[b] === 2 ? 1 : 0;  // Entrée avec résistance de tirage : 1 au repos
      },
      definir(b, v) { valeurs[b] = v; }
    };
  }

  ////////////////////////////////////////////////////////////////////////////////////
  // Le dessin de l'écran sur un canvas
  ////////////////////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////////////////////
  // La police du Minitel : chaque caractère est une matrice de 8 points sur 10 lignes
  // (STUM 1B, partie 1, chapitre 3, § 2.2.2). Ce sont les 128 caractères du jeu G0 du
  // processeur de visualisation EF9345, relevés par Frédéric Bisson pour MiEdit
  // (https://github.com/Zigazou/miedit, font/ef9345-g0.png) : 10 octets par caractère,
  // un par ligne, bit 7 à gauche. Les codes 0x20 à 0x7F sont ceux de l'ASCII ; les
  // codes 0x00 à 0x1F contiennent les lettres accentuées et quelques signes.
  ////////////////////////////////////////////////////////////////////////////////////

  const EF9345 =
    "003844402010001000000010280038447c4400000008103c2030203c0000000814103810243c0000" +
    "001028344c444c340000003844404040443810200028003c2030203c00000020180038447c440000" +
    "002018004444443800000010083c2030203c0000003c50505850503c00000008143c2030203c0000" +
    "000010207f20100000000010280030101038000000001008fc081000000010101010101054381000" +
    "001824180000000000000010107c1010007c000000081038447c4038000000280038447c40380000" +
    "00280030101010380000000000384040403810200010280044444c340000002010344c444c340000" +
    "000010007c001000000000201038447c403800000000003c525e503e000000102838447c40380000" +
    "0040c040444c143e04000040c0404c5204081e0000e0204024cc143e040000102800384444380000" +
    "0000000000000000000000101010101000100000002828280000000000000028287c287c28280000" +
    "001038503814543810000060640810204c0c00000020505020544834000000101020000000000000" +
    "000810202020100800000020100808081020000000105438103854100000000010107c1010000000" +
    "0000000000002020400000000000003c000000000000000000000020000001020204081020204080" +
    "0010284444442810000000103010101010100000003844041820407c0000007c0408180444380000" +
    "00081828487c08080000007c407804044438000000182040784444380000007c0408102020200000" +
    "00384444384444380000003844443c04043800000000002000000020000000000020000020204000" +
    "000408102010080400000000007c007c000000000040201008102040000000384404081000100000" +
    "0038445c545c4038000000384444447c444400000078444478444478000000384440404044380000" +
    "00784444444444780000007c40407040407c0000007c404070404040000000384440404c443c0000" +
    "004444447c444444000000381010101010380000001c080808084830000000444850605048440000" +
    "004040404040407c000000446c5444444444000000444464544c4444000000384444444444380000" +
    "00784444784040400000003844444454483400000078444478504844000000384440380444380000" +
    "007c1010101010100000004444444444443800000044444428281010000000444444545454280000" +
    "0044442810284444000000444428101010100000007c04081020407c0000001c10101010101c0000" +
    "804040201008040402010038080808080838000000103854101010101010000000000000000000ff" +
    "00000000ff0000000000000000344c444c3400000040407844444478000000000038404040380000" +
    "0004043c4444443c000000000038447c40380000001824207020202000000000003c44443c042418" +
    "00404058644444440000001000301010103800000008001808080808483000202024283028240000" +
    "00301010101010380000000000685454545400000000005864444444000000000038444444380000" +
    "000000784444447840400000003c4444443c04040000005864404040000000000038403804780000" +
    "002020382020201800000000004444444c3400000000004444282810000000000044445454280000" +
    "0000004428102844000000000044444c340444380000007c0810207c000080808080808080808080" +
    "1010101010101010101001010101010101010101ff000000000000000000ffffffffffffffffffff";
  // Sans dessin propre dans la police (ä, ë, î, ö, ü) : la lettre seule
  const EF9345_AUTRES = {'\u00eb': 0x65, '\u00a3': 0x03, '\u00b0': 0x10, '\u00b1': 0x11, '\u2190': 0x0C, '\u2191': 0x5E, '\u2192': 0x0E, '\u2193': 0x0F, '\u00bc': 0x1C, '\u00bd': 0x1D, '\u00be': 0x1E, '\u00e7': 0x15, '\u00c7': 0x05, '\u00e0': 0x17, '\u00c0': 0x07, '\u00e2': 0x04, '\u00c2': 0x01, '\u00e8': 0x19, '\u00c8': 0x09, '\u00e9': 0x12, '\u00c9': 0x02, '\u00ea': 0x1B, '\u00ca': 0x0B, '\u00ef': 0x14, '\u00f4': 0x1F, '\u00f9': 0x08, '\u00fb': 0x16, '\u0152': 0x0A, '\u0153': 0x1A, '\u00f7': 0x18, '\u00e4': 0x61, '\u00ee': 0x69, '\u00f6': 0x6F, '\u00fc': 0x75};

  let glyphesEF9345 = null;
  function glyphes() {
    if (!glyphesEF9345) {
      glyphesEF9345 = new Uint8Array(1280);
      for (let i = 0; i < 1280; i++) glyphesEF9345[i] = parseInt(EF9345.substr(i * 2, 2), 16);
    }
    return glyphesEF9345;
  }

  function indexEF9345(ch) {
    // Le caractère de la police qui dessine ch (? s'il n'y en a pas)
    const c = ch.codePointAt(0);
    if (c >= 0x20 && c < 0x80) return c;
    if (ch in EF9345_AUTRES) return EF9345_AUTRES[ch];
    return 0x3F;
  }

  // Niveaux de gris du Minitel 1B (0 noir, 1 rouge, 2 vert, 3 jaune, 4 bleu, 5 magenta, 6 cyan, 7 blanc)
  const GRIS = [0, 100, 160, 215, 70, 130, 190, 240];
  const PALETTE_GRIS = GRIS.map((g) => `rgb(${g},${g},${g})`);
  const PALETTE_COULEUR = ['#000000', '#e8262a', '#2ec43a', '#f2e530', '#2c50e8', '#d63ad6', '#2ed6dc', '#f2f2f2'];

  class Rendu {
    constructor(canvas, ecran, options) {
      this.canvas = canvas;
      this.ecran = ecran;
      this.echelle = options.echelle;
      this.palette = options.couleurs ? PALETTE_COULEUR : PALETTE_GRIS;
      this.police = options.police;
      this.bitmap = (options.police === 'minitel');
      this.sprites = {};
      this.phase = true;
      this.cw = 8 * this.echelle;
      this.ch = 10 * this.echelle;
      const dpr = (typeof window !== 'undefined' && window.devicePixelRatio) || 1;
      this.dpr = dpr;
      canvas.width = Math.round(40 * this.cw * dpr);
      canvas.height = Math.round(24 * this.ch * dpr);
      canvas.style.width = (40 * this.cw) + 'px';
      canvas.style.aspectRatio = '4 / 3';
      this.ctx = canvas.getContext('2d');
      this.ctx.imageSmoothingEnabled = false;
      this.ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
      this.taillePolice = Math.round(this.ch * 0.84);
      this.fonte = '700 ' + this.taillePolice + 'px ' + this.police;
      this.ctx.font = this.fonte;
      const m = this.ctx.measureText('M').width || this.cw * 0.6;
      this.etirement = (this.cw * 0.95) / m;
    }

    sprite(idx, couleur) {
      // Le dessin d'un caractère de la police (8 x 10 points), dans une couleur
      const cle = idx + couleur;
      let c = this.sprites[cle];
      if (c) return c;
      c = document.createElement('canvas');
      c.width = 8;
      c.height = 10;
      const g = c.getContext('2d');
      g.fillStyle = couleur;
      const p = glyphes();
      for (let y = 0; y < 10; y++) {
        const ligne = p[idx * 10 + y];
        for (let x = 0; x < 8; x++) if (ligne & (0x80 >> x)) g.fillRect(x, y, 1, 1);
      }
      this.sprites[cle] = c;
      return c;
    }

    dessiner(phase) {
      this.ctx.imageSmoothingEnabled = false;
      const ctx = this.ctx, cw = this.cw, ch = this.ch, pal = this.palette;
      ctx.fillStyle = pal[0];
      ctx.fillRect(0, 0, 40 * cw, 24 * ch);
      ctx.textAlign = 'center';
      ctx.textBaseline = 'middle';
      ctx.font = this.fonte;
      const grands = [];
      for (let r = 0; r < 24; r++) {
        for (let c = 0; c < 40; c++) {
          const x = this.ecran.cases[r][c];
          if (x.couverte) continue;
          if (!x.graphique && x.taille) { grands.push([r, c, x]); continue; }
          this.dessinerCase(r, c, x, phase);
        }
      }
      for (const [r, c, x] of grands) this.dessinerCase(r, c, x, phase);
      // Le curseur : un pavé qui clignote
      const e = this.ecran;
      if (e.curseur && phase) {
        ctx.globalCompositeOperation = 'difference';
        ctx.fillStyle = pal[7];
        ctx.fillRect(e.c * cw, e.r * ch, cw, ch);
        ctx.globalCompositeOperation = 'source-over';
      }
    }

    dessinerCase(r, c, x, phase) {
      const ctx = this.ctx, cw = this.cw, ch = this.ch, pal = this.palette;
      const px = c * cw, py = r * ch;
      let avant = pal[x.couleur], fond = pal[x.fond];
      const visible = !x.clignote || phase;
      if (x.graphique) {
        ctx.fillStyle = fond;
        ctx.fillRect(px, py, cw, ch);
        if (!visible) return;
        ctx.fillStyle = avant;
        const ys = [0, Math.round(ch * 0.35), Math.round(ch * 0.65), ch];
        const demi = cw / 2;
        for (let by = 0; by < 3; by++) {
          for (let bx = 0; bx < 2; bx++) {
            if (x.points & (32 >> (by * 2 + bx))) ctx.fillRect(px + bx * demi, py + ys[by], demi, ys[by + 1] - ys[by]);
          }
        }
        return;
      }
      if (x.inverse) { const t = avant; avant = fond; fond = t; }
      const larg = (x.taille & 2) ? 2 : 1, haut = (x.taille & 1) ? 2 : 1;
      const w = cw * larg, h = ch * haut;
      const top = py - ch * (haut - 1);
      ctx.fillStyle = fond;
      ctx.fillRect(px, top, w, h);
      if (!visible) return;
      ctx.fillStyle = avant;
      if (this.bitmap) {
        const idx = indexEF9345(x.car);
        if (idx !== 0x20) ctx.drawImage(this.sprite(idx, avant), px, top, w, h);
        if (x.souligne) ctx.fillRect(px, top + h - (ch / 10) * haut, w, (ch / 10) * haut);
        return;
      }
      if (x.car !== ' ') {
        ctx.save();
        ctx.translate(px + w / 2, top + h / 2 + ch * 0.04 * haut);
        ctx.scale(this.etirement * larg, haut);
        ctx.fillText(x.car, 0, 0);
        ctx.restore();
      }
      if (x.souligne) ctx.fillRect(px, top + h - Math.max(1, Math.round(ch / 10)) * haut, w, Math.max(1, Math.round(ch / 10)) * haut);
    }
  }

  ////////////////////////////////////////////////////////////////////////////////////
  // La machine : l'écran, le clavier et l'interpréteur ensemble
  ////////////////////////////////////////////////////////////////////////////////////

  const STYLE = `
.rebasic{display:inline-flex;flex-direction:column;gap:10px;align-items:stretch;max-width:100%;font-family:system-ui,sans-serif}
.rebasic-ecran{position:relative;background:#000;border-radius:14px;padding:18px;box-shadow:inset 0 0 30px rgba(0,0,0,.9),0 0 0 6px #2a2622;cursor:text;outline:none;line-height:0}
.rebasic-ecran:focus-within{box-shadow:inset 0 0 30px rgba(0,0,0,.9),0 0 0 6px #2a2622,0 0 0 9px #8a7a63}
.rebasic-ecran canvas{display:block;max-width:100%;height:auto;image-rendering:auto}
.rebasic-saisie{position:absolute;left:0;top:0;width:1px;height:1px;opacity:0;border:0;padding:0;resize:none;overflow:hidden;font-size:16px}
.rebasic-touches{display:flex;flex-wrap:wrap;gap:6px;justify-content:center}
.rebasic-touches button{font:600 11px/1 system-ui,sans-serif;letter-spacing:.04em;padding:8px 9px;border-radius:5px;border:1px solid #6d6457;background:#d9d1c3;color:#2b2620;cursor:pointer;box-shadow:0 2px 0 #8d8270}
.rebasic-touches button:active{transform:translateY(1px);box-shadow:0 1px 0 #8d8270}
.rebasic-touches button.rebasic-envoi{background:#2b2620;color:#efe8da;border-color:#2b2620;box-shadow:0 2px 0 #000}
`;

  ////////////////////////////////////////////////////////////////////////////////////
  // Le clavier virtuel, d'après celui du Minitel 1B
  ////////////////////////////////////////////////////////////////////////////////////

  const STYLE_CLAVIER = `
.rebasic-clavier-cadre{container-type:inline-size;width:100%}
.rebasic-clavier{--u:40px;--u:calc(100cqw / 16.6);--g:calc(var(--u) * .2);background:linear-gradient(175deg,#4e3e37,#3e312c 60%,#372b26);border-radius:6px 6px 18px 18px;padding:calc(var(--u) * .55) calc(var(--u) * .35) calc(var(--u) * .4);box-shadow:inset 0 1px 0 #ffffff1c,0 10px 24px -12px #000a;font-family:"Helvetica Neue",Helvetica,Arial,sans-serif;user-select:none;-webkit-user-select:none;touch-action:manipulation}
.rebasic-clavier .rc-haut{display:flex;justify-content:space-between;align-items:flex-start;margin-bottom:calc(var(--u) * .55)}
.rebasic-clavier .rc-gauche{display:flex;flex-direction:column;gap:calc(var(--u) * 1.47);padding-top:calc(var(--u) * .1)}
.rebasic-clavier .rc-fonctions{display:grid;grid-template-columns:repeat(4,auto);gap:calc(var(--u) * .62) calc(var(--u) * .3);margin-top:calc(var(--u) * .95)}
.rebasic-clavier .rc-pave{display:grid;grid-template-columns:repeat(3,var(--u));gap:calc(var(--u) * .5) calc(var(--u) * .45);margin-top:calc(var(--u) * .1)}
.rebasic-clavier .rc-rang{display:flex;gap:var(--g);margin-top:calc(var(--u) * .5)}
.rebasic-clavier .rc-rang.serre{margin-top:var(--g)}
.rebasic-clavier .rk{position:relative;flex:none;width:var(--u);height:var(--u);padding:0;border:0;border-radius:calc(var(--u) * .14);background:linear-gradient(#34302d,#1a1716 70%);color:#f3eee6;font:700 calc(var(--u) * .4)/1 "Helvetica Neue",Helvetica,Arial,sans-serif;box-shadow:0 calc(var(--u) * .08) 0 #0b0908,0 calc(var(--u) * .1) calc(var(--u) * .12) #0008,inset 0 1px 0 #ffffff26;cursor:pointer;display:flex;align-items:center;justify-content:center}
.rebasic-clavier .rk:active,.rebasic-clavier .rk.rc-appui{transform:translateY(calc(var(--u) * .06));box-shadow:0 calc(var(--u) * .02) 0 #0b0908,inset 0 1px 0 #ffffff1a;filter:brightness(1.25)}
.rebasic-clavier .rk.rc-fn{width:calc(var(--u) * 1.95);height:calc(var(--u) * .62);background:linear-gradient(#cd8268,#ab5d44 75%);color:#fff;font-weight:500;font-size:calc(var(--u) * .27);box-shadow:0 calc(var(--u) * .07) 0 #5d2c1d,inset 0 1px 0 #ffffff40}
.rebasic-clavier .rk.rc-envoi{background:linear-gradient(#8f9a4d,#6b7530 75%);box-shadow:0 calc(var(--u) * .07) 0 #39401a,inset 0 1px 0 #ffffff40}
.rebasic-clavier .rk.rc-petit{width:calc(var(--u) * 1.3);height:calc(var(--u) * .62);font-size:calc(var(--u) * .25)}
.rebasic-clavier .rk.rc-esc{font-size:calc(var(--u) * .27);font-weight:500}
.rebasic-clavier .rk.rc-ctrl{font-size:calc(var(--u) * .27);font-weight:500;outline:2px solid #c9472d;outline-offset:calc(var(--u) * .07)}
.rebasic-clavier .rk.rc-maj{width:calc(var(--u) * 1.15);outline:2px solid #d79b35;outline-offset:calc(var(--u) * .07)}
.rebasic-clavier .rk.rc-actif{background:linear-gradient(#5a524c,#3a3330 70%);outline-color:#ffd27a}
.rebasic-clavier .rk.rc-verrou::after{content:"";position:absolute;top:calc(var(--u) * .12);right:calc(var(--u) * .12);width:calc(var(--u) * .12);height:calc(var(--u) * .12);border-radius:50%;background:#ffb547;box-shadow:0 0 6px #ffb547}
.rebasic-clavier .rk.rc-espace{width:calc(var(--u) * 6.4)}
.rebasic-clavier .rk.rc-entree{width:calc(var(--u) * 1.5)}
.rebasic-clavier .rk .rc-o,.rebasic-clavier .rk .rc-r{position:absolute;bottom:calc(100% + var(--u) * .1);font-size:calc(var(--u) * .32);font-weight:600;line-height:1;white-space:nowrap;pointer-events:none}
.rebasic-clavier .rk .rc-o{left:calc(var(--u) * -.06);color:#e2ac2e;transform:translate(calc(-50% + var(--dx, 0px)), var(--dy, 0px))}
.rebasic-clavier .rc-pave .rk .rc-o{bottom:calc(100% + var(--u) * .03)}
.rebasic-clavier .rc-pave .rk .rc-o.rc-accent,.rebasic-clavier .rc-pave .rk .rc-o.rc-moyen{bottom:calc(100% + var(--u) * .03)}
.rebasic-clavier .rk .rc-r{right:calc(var(--u) * -.04);color:#ea5a2c;font-size:calc(var(--u) * .23)}
.rebasic-clavier .rc-bas .rk .rc-o,.rebasic-clavier .rc-bas .rk .rc-r{font-size:calc(var(--u) * .25)}
.rebasic-clavier .rc-bas .rk .rc-o{left:calc(var(--u) * -.08);transform:none}
.rebasic-clavier .rk .rc-o.rc-accent{font-size:calc(var(--u) * .55);bottom:calc(100% - var(--u) * .1)}
.rebasic-clavier .rk .rc-o.rc-moyen{font-size:calc(var(--u) * .48);bottom:calc(100% + var(--u) * .02)}
.rebasic-clavier .rk .rc-o.rc-etoile{bottom:calc(100% + var(--u) * .137)}
.rebasic-clavier .rk .rc-o.rc-etoile svg{display:block;width:calc(var(--u) * .24);height:calc(var(--u) * .24)}
.rebasic-clavier .rk .rc-o.rc-trait{bottom:calc(100% + var(--u) * .19)}
.rebasic-clavier .rk .rc-o.rc-trait svg{display:block;width:calc(var(--u) * .15);height:calc(var(--u) * .19)}
.rebasic-clavier .rk.rc-fn .rc-r{right:calc(var(--u) * .12)}
.rebasic-clavier .rk .rc-l{font-size:calc(var(--u) * .25);line-height:1.05;text-align:center;font-weight:500}
`;

  const CONNEXION_FIN = 0x1349;
  const VDT_DUREE = 10000;  // Défilement automatique des pages Vidéotex : 10 s par page
  const ACCENTS_GRAVES = ['\u00b4', '`', '\u00a8', '"'];
  const SIGNES_MOYENS = ['\u2019'];  // Gravés plus grands que leur dessin dans la police
  // Signes que la police dessine trop haut : on les descend (en fractions de touche) pour que
  // le milieu de chaque signe soit sur la même ligne que ses voisins
  const DESCENTES = { '\u00a8': .11, '@': -.01, '/': .012, '"': .227, '\u2019': -.092, '(': -.012, ')': -.012, '[': -.012, ']': -.012, '\u2191': .008 };
  const CTRL_NOMS = { I: 'TAB', H: 'BS', J: 'LF', X: 'CAN' };

  function creerClavierVirtuel(machine) {
    const cadre = document.createElement('div');
    cadre.className = 'rebasic-clavier-cadre';
    const clavier = document.createElement('div');
    clavier.className = 'rebasic-clavier';
    clavier.setAttribute('role', 'group');
    clavier.setAttribute('aria-label', 'Clavier du Minitel');
    cadre.appendChild(clavier);

    let maj = 0;        // 0 : non, 1 : pour une touche, 2 : verrouillée
    let ctrl = false;
    const parCode = {}; // Touche du clavier virtuel qui correspond à un code (pour l'allumer)
    const majs = [];
    let ctrlTouche = null;

    function etat() {
      for (const t of majs) {
        t.classList.toggle('rc-actif', maj > 0);
        t.classList.toggle('rc-verrou', maj === 2);
      }
      if (ctrlTouche) ctrlTouche.classList.toggle('rc-actif', ctrl);
    }

    function allumer(el) {
      el.classList.add('rc-appui');
      setTimeout(() => el.classList.remove('rc-appui'), 110);
    }

    // L'apostrophe du clavier : une tête ronde et une queue légèrement courbée (largeur et
    // hauteur en touches)
    function apostrophe(l, h, style) {
      return '<svg viewBox="0 0 8 12" aria-hidden="true" style="display:block;width:calc(var(--u) * ' + l +
        ');height:calc(var(--u) * ' + h + ')' + (style ? ';' + style : '') + '">' +
        '<circle cx="4.6" cy="3" r="1.9" fill="currentColor"/>' +
        '<path d="M6.3 3.4 Q6.1 7.4 3.4 10.2" fill="none" stroke="currentColor" stroke-width="1.6" stroke-linecap="round"/></svg>';
    }

    // La ponctuation gravée sur les touches , . ; : : tous les points du bas (le point, la tête
    // de la virgule) sont au même endroit, et le point du haut de ; et : aussi
    function ponctuation(car) {
      const point = (y) => '<circle cx="4.6" cy="' + y + '" r="1.9" fill="currentColor"/>';
      const queue = '<path d="M6.3 11.4 Q6.1 15.4 3.4 18.2" fill="none" stroke="currentColor" stroke-width="1.6" stroke-linecap="round"/>';
      const haut = car === ';' || car === ':' ? point(3) : '';
      const bas = point(11) + (car === ',' || car === ';' ? queue : '');
      return '<svg viewBox="0 0 8 20" aria-hidden="true" style="display:block;width:calc(var(--u) * .16);' +
        'height:calc(var(--u) * .4);transform:translateY(calc(var(--u) * .08))">' + haut + bas + '</svg>';
    }

    function touche(contenu, action, opt) {
      opt = opt || {};
      const b = document.createElement('button');
      b.type = 'button';
      b.className = 'rk' + (opt.cls ? ' ' + opt.cls : '');
      if (opt.html) b.innerHTML = contenu;
      else b.textContent = contenu;
      if (opt.orange) {
        // Les accents et les apostrophes sont gravés en grand : on les agrandit pour qu'ils se voient
        const s = document.createElement('span');
        s.className = 'rc-o' + (ACCENTS_GRAVES.includes(opt.orange) ? ' rc-accent' : '') +
          (SIGNES_MOYENS.includes(opt.orange) ? ' rc-moyen' : '') +
          (opt.orange === '*' ? ' rc-etoile' : '') +  // L'astérisque est dessiné : voir plus bas
          (opt.orange === '\u00b4' || opt.orange === '`' || opt.orange === '^' ? ' rc-trait' : '');
        s.textContent = opt.orange;
        if (DESCENTES[opt.orange]) s.style.setProperty('--dy', 'calc(var(--u) * ' + DESCENTES[opt.orange] + ')');
        if (opt.orange === '\u2019') s.style.setProperty('--dx', 'calc(var(--u) * -.025)');  // Apostrophe dessinée penchée
        if (opt.orange === '*') {
          // L'astérisque de la police est petit et placé trop haut, différemment selon la
          // résolution de l'écran : on le dessine (six branches, comme sur le clavier), centré
          // sur la même ligne que ses voisins
          s.innerHTML = '<svg viewBox="0 0 10 10" aria-hidden="true"><path d="M5 1 L5 9 M1.5 3 L8.5 7 M1.5 7 L8.5 3"' +
            ' fill="none" stroke="currentColor" stroke-width="1.7" stroke-linecap="round"/></svg>';
        }
        if (s.classList.contains('rc-trait')) {
          // Accent aigu ou grave : un petit trait penché et légèrement courbé, comme sur le clavier
          // (l'accent circonflexe : un chevron fin, de la hauteur des autres accents)
          const d = opt.orange === '^' ? 'M1.2 7.6 L4 2.6 L6.8 7.6' :
            opt.orange === '`' ? 'M1.8 2 Q3.1 5.1 6 8' : 'M6.2 2 Q4.9 5.1 2 8';
          s.innerHTML = '<svg viewBox="0 0 8 10" aria-hidden="true"><path d="' + d +
            '" fill="none" stroke="currentColor" stroke-width="1.9" stroke-linecap="round" stroke-linejoin="round"/></svg>';
        }
        if (opt.orange === '\u2019') {
          // Apostrophe : une tête ronde et une queue courbée, comme sur le clavier
          s.innerHTML = apostrophe('.13', '.2');
        }
        b.appendChild(s);
      }
      if (opt.rouge) { const s = document.createElement('span'); s.className = 'rc-r'; s.textContent = opt.rouge; b.appendChild(s); }
      if (opt.titre) b.title = opt.titre;
      b.addEventListener('mousedown', (e) => e.preventDefault());
      b.addEventListener('click', () => {
        if (machine.audio) machine.audio.debloquer();
        action();
        // Sur un écran tactile, on n'ouvre pas le clavier du téléphone
        const tactile = typeof matchMedia === 'function' && matchMedia('(pointer: coarse)').matches;
        if (!tactile) machine.focus();
      });
      if (opt.code) parCode[opt.code] = b;
      return b;
    }

    function envoyer(code) {
      machine.touches.push(code);
      machine.reveiller();
    }

    // Une touche de caractère : car en majuscule, autre avec la touche majuscule
    function caractere(car, autre, opt) {
      opt = opt || {};
      const lettre = /^[A-Z]$/.test(car);
      const b = touche(car, () => {
        let c = car;
        if (lettre) {
          if (ctrl) { envoyer(car.charCodeAt(0) & 0x1F); ctrl = false; if (maj === 1) maj = 0; etat(); return; }
          if (maj > 0) c = car.toLowerCase();
        }
        else if (maj > 0 && autre) c = autre;
        envoyer(c.codePointAt(0));
        if (maj === 1) maj = 0;
        ctrl = false;
        etat();
      }, Object.assign({ orange: lettre ? null : autre === '^' ? '\u2191' : autre === "'" ? '\u2019' : autre, rouge: lettre ? CTRL_NOMS[car] : null }, opt));
      parCode[car.charCodeAt(0)] = b;
      if (lettre) parCode[car.toLowerCase().charCodeAt(0)] = b;
      if (autre) parCode[autre.charCodeAt(0)] = b;
      return b;
    }

    function fonction(nom, code, cls, orange, rouge) {
      return touche(nom, () => { envoyer(code); maj = maj === 2 ? 2 : 0; ctrl = false; etat(); }, { cls: 'rc-fn' + (cls ? ' ' + cls : ''), code, orange, rouge });
    }

    // Le haut : Connexion/Fin et Fnct (Fnct à la hauteur de Guide), les touches de fonction
    // avec les caractères gravés au-dessus, le pavé numérique
    const haut = document.createElement('div');
    haut.className = 'rc-haut';
    const gauche = document.createElement('div');
    gauche.className = 'rc-gauche';
    gauche.appendChild(touche('<span class="rc-l">Connexion<br>Fin</span>', () => envoyer(CONNEXION_FIN), { cls: 'rc-fn rc-petit', html: true, rouge: 'BRK', code: CONNEXION_FIN }));
    gauche.appendChild(touche('Fnct', () => { }, { cls: 'rc-fn rc-petit', titre: 'Fnct' }));
    haut.appendChild(gauche);

    const fonctions = document.createElement('div');
    fonctions.className = 'rc-fonctions';
    for (const [nom, code, cls, orange, rouge] of [
      ['Sommaire', SOMMAIRE, '', '^'], ['Annulation', ANNULATION, '', '\\', '\u00a3'],
      ['Retour', RETOUR, '', '\u00b4', '\u0152'], ['R\u00e9p\u00e9tition', REPETITION, '', '{', '\u0153'],
      ['Guide', GUIDE, '', '\u00a8'], ['Correction', CORRECTION, '', '\u00a7', '\u00e7'],
      ['Suite', SUITE, '', '`', '\u00df'], ['Envoi', ENVOI, 'rc-envoi', '}']
    ]) fonctions.appendChild(fonction(nom, code, cls, orange, rouge));
    haut.appendChild(fonctions);

    const pave = document.createElement('div');
    pave.className = 'rc-pave';
    for (const [car, autre] of [['1', '!'], ['2', '"'], ['3', '#'], ['4', '$'], ['5', '%'], ['6', '&'],
      ['7', "'"], ['8', '('], ['9', ')'], ['*', '['], ['0', '^'], ['#', ']']]) {
      const k = caractere(car, autre);
      if (car === '*') {
        // L'astérisque de la police est petit et dessiné trop haut : on l'agrandit à la taille
        // de # et on le centre comme 0 et #
        const t = document.createElement('span');
        t.textContent = '*';
        t.style.fontSize = 'calc(var(--u) * .72)';
        t.style.transform = 'translateY(calc(var(--u) * .156))';
        k.replaceChild(t, k.firstChild);
      }
      pave.appendChild(k);
    }
    haut.appendChild(pave);
    clavier.appendChild(haut);

    const rang = (decalage, serre) => {
      const r = document.createElement('div');
      r.className = 'rc-rang' + (serre ? ' serre' : '');
      r.style.marginLeft = 'calc(var(--u) * ' + decalage + ')';
      clavier.appendChild(r);
      return r;
    };

    // La ponctuation
    let r = rang(2.15);
    r.appendChild(touche('Esc', () => { envoyer(0x1B); ctrl = false; etat(); }, { cls: 'rc-esc', code: 0x1B }));
    for (const [car, autre] of [[',', '<'], ['.', '>'], ["'", '@'], [';', '+'], ['-', '='], [':', '*'], ['?', '/']]) {
      const k = caractere(car, autre);
      // L'apostrophe, la virgule et le point-virgule gravés sur les touches, dessinés comme
      // l'apostrophe au-dessus du 7 (la virgule en bas, l'apostrophe en haut)
      const dessin = car === "'" ? apostrophe('.16', '.24', 'transform:translateY(calc(var(--u) * -.07))') :
        ',.;:'.includes(car) ? ponctuation(car) : null;
      if (dessin) {
        const t = document.createElement('span');
        t.innerHTML = dessin;
        k.replaceChild(t.firstChild, k.firstChild);
      }
      r.appendChild(k);
    }

    // Les lettres (AZERTY)
    r = rang(1.15);
    for (const c of 'AZERTYUIOP') r.appendChild(caractere(c));
    // TAB paraît trop haut sous la rangée de ponctuation : on le rapproche du I
    parCode[0x49].querySelector('.rc-r').style.bottom = 'calc(100% + var(--u) * .08)';
    r = rang(0.15);
    ctrlTouche = touche('Ctrl', () => { ctrl = !ctrl; etat(); }, { cls: 'rc-ctrl', titre: 'Ctrl' });
    r.appendChild(ctrlTouche);
    for (const c of 'QSDFGHJKLM') r.appendChild(caractere(c));
    r = rang(0.75);
    const majuscule = () => touche('', () => { maj = (maj + 1) % 3; etat(); }, { cls: 'rc-maj', titre: 'Majuscule (deux fois : verrouill\u00e9e)' });
    const m1 = majuscule();
    majs.push(m1);
    r.appendChild(m1);
    for (const c of 'WXCVBN') r.appendChild(caractere(c));
    const m2 = majuscule();
    majs.push(m2);
    r.appendChild(m2);

    // Les flèches, l'espace et le retour chariot : écritures orange et rouges de même taille
    r = rang(0.15);
    r.classList.add('rc-bas');
    r.appendChild(touche('\u2191', () => envoyer(FLECHE_HAUT), { orange: 'Sup. L', code: FLECHE_HAUT }));
    r.appendChild(touche('\u2193', () => envoyer(FLECHE_BAS), { orange: 'Ins. L', code: FLECHE_BAS }));
    const espace = touche('', () => { envoyer(0x20); if (maj === 1) maj = 0; ctrl = false; etat(); }, { cls: 'rc-espace', titre: 'Espace' });
    parCode[0x20] = espace;
    r.appendChild(espace);
    const fg = touche('\u2190', () => envoyer(FLECHE_GAUCHE), { orange: 'Sup. C', code: FLECHE_GAUCHE });
    r.appendChild(fg);
    const fd = touche('\u2192', () => envoyer(FLECHE_DROITE), { orange: 'Ins. C', rouge: 'DEL', code: FLECHE_DROITE });
    fd.querySelector('.rc-o').style.left = 'calc(var(--u) * .25)';
    fd.querySelector('.rc-r').style.right = 'auto';
    fd.querySelector('.rc-r').style.left = 'calc(var(--u) * -.38)';
    r.appendChild(fd);
    const entree = touche('\u21b5', () => envoyer(0x0D), { cls: 'rc-entree', orange: '_', rouge: 'E. Page', titre: 'Retour chariot' });
    entree.style.marginLeft = 'calc(var(--u) * .9)';
    // Le symbole gravé au-dessus de la touche : un trait qui remonte vers la gauche en flèche
    entree.querySelector('.rc-o').innerHTML = '<svg viewBox="0 0 20 8" style="display:block;width:calc(var(--u) * .5);height:calc(var(--u) * .2)" aria-hidden="true">' +
      '<path d="M19 6.5 L5.5 6.5 L2.8 3" fill="none" stroke="currentColor" stroke-width="1.6" stroke-linecap="round" stroke-linejoin="round"/>' +
      '<path d="M0.97 0.62 L4.07 2.02 L1.53 3.98 Z" fill="currentColor"/></svg>';
    entree.querySelector('.rc-o').style.left = 'calc(var(--u) * -.3)';
    parCode[0x0D] = entree;
    r.appendChild(entree);

    etat();
    return {
      element: cadre,
      allumer(code) { if (parCode[code]) allumer(parCode[code]); }
    };
  }

  const CLAVIER = {
    'Enter': ENVOI, 'Backspace': CORRECTION, 'Escape': ANNULATION,
    'F1': GUIDE, 'F2': SOMMAIRE, 'F3': REPETITION,
    'PageDown': SUITE, 'PageUp': RETOUR,
    'ArrowUp': FLECHE_HAUT, 'ArrowDown': FLECHE_BAS, 'ArrowLeft': FLECHE_GAUCHE, 'ArrowRight': FLECHE_DROITE
  };

  function peutChoisirEmplacement() {
    // Le navigateur peut-il ouvrir une fenêtre « Enregistrer sous » ? (Chrome, Edge, Opera,
    // et seulement si la page n'est pas dans un cadre d'un autre site)
    if (typeof window === 'undefined' || typeof window.showSaveFilePicker !== 'function') return false;
    try { return window.self === window.top; }
    catch (e) { return false; }
  }

  function dansUnCadre() {
    try { return typeof window !== 'undefined' && window.self !== window.top; }
    catch (e) { return true; }
  }

  const TYPES_FICHIERS = [
    { description: 'Programme reBASIC', accept: { 'text/plain': ['.bas', '.BAS', '.txt'] } },
    { description: 'Page Vid\u00e9otex', accept: { 'application/octet-stream': ['.vdt', '.VDT'] } }
  ];

  const BOUTONS = [
    ['SOMMAIRE', SOMMAIRE], ['ANNULATION', ANNULATION], ['RETOUR', RETOUR], ['R\u00c9P\u00c9TITION', REPETITION],
    ['GUIDE', GUIDE], ['CORRECTION', CORRECTION], ['SUITE', SUITE], ['ENVOI', ENVOI]
  ];

  function codeTouche(t) {
    if (typeof t === 'number') return t;
    const n = String(t).toUpperCase().normalize('NFD').replace(/[\u0300-\u036f]/g, '');
    if (n in TOUCHES) return TOUCHES[n];
    if (n === 'ANNUL') return ANNULATION;
    if (n === 'SOMM') return SOMMAIRE;
    if (n === 'CORR') return CORRECTION;
    return 0;
  }

  function caractereAccepte(ch) {
    const c = ch.codePointAt(0);
    return (c >= 0x20 && c < 0x7F) || !!G2[ch];
  }

  class Machine {
    constructor(conteneur, options) {
      this.options = Object.assign({
        echelle: 2,
        couleurs: false,
        vitesse: 4800,           // Bauds de la liaison avec le Minitel (0 : instantané)
        instructions: 1000,      // Instructions par seconde (0 : aussi vite que possible)
        carte: 'mega',           // 'mega' ou 'esp32' (tailles de la mémoire)
        menu: true,              // Démarrer sur l'écran d'accueil de rebootMinitel (menu, date et heure)
        version: VERSION,        // Version de rebootMinitel affichée sur l'écran d'accueil
        accueil: true,           // Afficher l'écran d'accueil de reBASIC
        afficherOk: true,        // Afficher « Ok » quand reBASIC attend
        programme: null,         // Programme chargé au démarrage
        lancer: false,           // Lancer ce programme (RUN) au démarrage
        clavier: true,           // Taper au clavier de l'ordinateur
        clavierPage: false,      // Les touches arrivent au Minitel même si l'écran n'a pas le focus
                                 // (sauf quand on tape dans un champ de la page)
        boutons: true,           // Touches de fonction du Minitel sous l'écran
        clavierVirtuel: false,   // Clavier complet du Minitel 1B sous l'écran (remplace les boutons)
        fichiers: false,         // Boutons « Ouvrir… » et « Enregistrer sous… » sous l'écran
        son: true,
        volume: 0.08,
        stockage: 'local',       // Carte SD : 'local' (localStorage) ou 'memoire'
        prefixe: 'rebasic:',
        police: 'minitel',       // 'minitel' (police de l'EF9345) ou une police CSS à chasse fixe
        horloge: null,           // Fonction qui renvoie la date (Date) de l'horloge
        surSommaire: null,       // Appelée quand on quitte reBASIC avec SOMMAIRE
        surBroche: null          // (genre, broche, valeur) : PINMODE, DWRITE, AWRITE, DREAD, AREAD
      }, options || {});
      const o = this.options;

      this.ecran = new Ecran();
      this.file = [];
      this.tete = 0;
      this.touches = [];
      this.commandes = [];
      this.attente = null;
      this.prochain = null;
      this.dernierDebit = maintenant();
      this.detruite = false;
      this.audio = o.son ? creerSon(o.volume) : null;
      this.sd = creerCarteSD(o.stockage, o.prefixe);
      this.broches = creerBroches(o.surBroche);
      this.ecran.surBip = () => { if (this.audio) this.audio.bip(); };

      const m = this;
      this.decalageHorloge = 0;
      try {
        const d = window.localStorage.getItem(o.prefixe.replace(/:$/, '') + '-horloge');
        if (d) this.decalageHorloge = Number(d) || 0;
      }
      catch (e) { }
      this.env = {
        carte: o.carte,
        octet(b) { m.file.push(b); },
        cle() { return m.touches.length ? m.touches.shift() : 0; },
        millis: () => maintenant(),
        horloge: () => new Date((o.horloge ? o.horloge().getTime() : Date.now()) + m.decalageHorloge),
        reglerHorloge: (d) => {
          m.decalageHorloge = d.getTime() - (o.horloge ? o.horloge().getTime() : Date.now());
          try { window.localStorage.setItem(o.prefixe.replace(/:$/, '') + '-horloge', String(m.decalageHorloge)); }
          catch (e) { }
        },
        versionReboot: o.version,
        accueilBasic: o.accueil,
        surSommaire: () => { if (o.surSommaire) o.surSommaire(m); },
        horlogeOK: true,
        hasard: Math.random,
        son: (f, d) => { if (m.audio) m.audio.son(f, d); },
        sonStop: () => { if (m.audio) m.audio.arreter(); },
        sd: this.sd,
        broches: this.broches,
        fileOctets: () => m.file.length - m.tete,
        cadence: () => o.instructions,
        afficherOk: o.afficherOk,
        commandeEnAttente: () => (m.commandes.length ? m.commandes[0] : null),
        commandeFaite: () => { m.commandes.shift(); }
      };
      this.interp = creerInterpreteur(this.env);

      if (conteneur) this.monter(conteneur);
      if (o.programme) this.charger(o.programme);
      if (o.lancer) this.lancer();
      this.demarrer(o.accueil);
    }

    monter(conteneur) {
      const o = this.options;
      if (typeof document !== 'undefined' && !document.getElementById('rebasic-style')) {
        const s = document.createElement('style');
        s.id = 'rebasic-style';
        s.textContent = STYLE + STYLE_CLAVIER;
        document.head.appendChild(s);
      }
      this.racine = document.createElement('div');
      this.racine.className = 'rebasic';
      const cadre = document.createElement('div');
      cadre.className = 'rebasic-ecran';
      const canvas = document.createElement('canvas');
      canvas.setAttribute('role', 'img');
      canvas.setAttribute('aria-label', '\u00c9cran du Minitel');
      cadre.appendChild(canvas);
      this.racine.appendChild(cadre);
      this.rendu = new Rendu(canvas, this.ecran, o);
      this.canvas = canvas;

      if (o.clavier) {
        const saisie = document.createElement('textarea');
        saisie.className = 'rebasic-saisie';
        saisie.setAttribute('autocapitalize', 'off');
        saisie.setAttribute('autocomplete', 'off');
        saisie.setAttribute('autocorrect', 'off');
        saisie.setAttribute('spellcheck', 'false');
        saisie.setAttribute('aria-label', 'Clavier du Minitel');
        cadre.appendChild(saisie);
        this.saisie = saisie;
        cadre.addEventListener('mousedown', (e) => { e.preventDefault(); this.focus(); });
        cadre.addEventListener('touchend', () => this.focus());
        saisie.addEventListener('keydown', (e) => {
          if (this.audio) this.audio.debloquer();
          const code = CLAVIER[e.key];
          if (code && !e.altKey && !e.metaKey) {
            e.preventDefault();
            this.touche(code);
          }
        });
        let composition = false;
        saisie.addEventListener('compositionstart', () => { composition = true; });
        saisie.addEventListener('compositionend', () => { composition = false; vider(); });
        const vider = () => {
          const v = saisie.value;
          saisie.value = '';
          for (const ch of v) {
            if (ch === '\n') this.touche(ENVOI);
            else if (caractereAccepte(ch)) this.touche(ch.codePointAt(0));
          }
        };
        saisie.addEventListener('input', () => { if (!composition) vider(); });

        if (o.clavierPage) {
          // Une touche frappée ailleurs dans la page va au Minitel, sauf dans un champ de saisie
          this.surTouchePage = (e) => {
            if (e.defaultPrevented || e.ctrlKey || e.metaKey || e.altKey) return;
            const a = document.activeElement;
            if (a === saisie) return;
            if (a && (a.isContentEditable || /^(INPUT|TEXTAREA|SELECT)$/.test(a.tagName))) return;
            if (this.audio) this.audio.debloquer();
            const code = CLAVIER[e.key];
            if (code) {
              e.preventDefault();
              this.touche(code);
              saisie.focus({ preventScroll: true });
            }
            else if (e.key.length === 1) {
              e.preventDefault();
              if (caractereAccepte(e.key)) this.touche(e.key.codePointAt(0));
              saisie.focus({ preventScroll: true });
            }
          };
          document.addEventListener('keydown', this.surTouchePage);
        }
      }

      if (o.clavierVirtuel) {
        this.clavierVirtuel = creerClavierVirtuel(this);
        this.racine.appendChild(this.clavierVirtuel.element);
      }
      else if (o.boutons) {
        const barre = document.createElement('div');
        barre.className = 'rebasic-touches';
        for (const [nom, code] of BOUTONS) {
          const b = document.createElement('button');
          b.type = 'button';
          b.textContent = nom;
          if (code === ENVOI) b.className = 'rebasic-envoi';
          b.addEventListener('mousedown', (e) => e.preventDefault());
          b.addEventListener('click', () => { if (this.audio) this.audio.debloquer(); this.touche(code); this.focus(); });
          barre.appendChild(b);
        }
        this.racine.appendChild(barre);
      }

      if (o.fichiers) {
        const barre = document.createElement('div');
        barre.className = 'rebasic-touches rebasic-fichiers';
        const bouton = (texte, action) => {
          const b = document.createElement('button');
          b.type = 'button';
          b.textContent = texte;
          b.addEventListener('click', () => {
            action().catch((e) => { if (typeof console !== 'undefined') console.error(e); });
          });
          barre.appendChild(b);
        };
        bouton('Ouvrir\u2026', () => this.ouvrirFichier());
        bouton('Enregistrer sous\u2026', () => this.enregistrerSous());
        this.racine.appendChild(barre);
      }

      conteneur.appendChild(this.racine);
      const boucle = () => {
        if (this.detruite) return;
        this.debiter();
        const phase = Math.floor(maintenant() / 500) % 2 === 0;
        if (this.ecran.modifie || phase !== this.phase) {
          this.ecran.modifie = false;
          this.phase = phase;
          this.rendu.dessiner(phase);
        }
        requestAnimationFrame(boucle);
      };
      requestAnimationFrame(boucle);
    }

    demarrer(accueil) {
      this.gen = this.options.menu ? this.interp.systeme() : this.interp.basic(accueil);
      this.attente = null;
      this.planifier(0);
    }

    // Les octets envoyés au Minitel arrivent à la vitesse de la liaison série
    // (7 bits + parité + start + stop : 10 bits par octet)
    debiter() {
      const t = maintenant();
      const v = this.options.vitesse;
      let n;
      if (!v) n = this.file.length - this.tete;
      else {
        n = Math.floor((t - this.dernierDebit) * v / 10000);
        if (n <= 0) {
          if (this.tete >= this.file.length) this.dernierDebit = t;
          return;
        }
        this.dernierDebit += n * 10000 / v;
      }
      while (n-- > 0 && this.tete < this.file.length) this.ecran.recevoir(this.file[this.tete++]);
      if (this.tete >= this.file.length) {
        this.file.length = 0;
        this.tete = 0;
        this.dernierDebit = t;
      }
      else if (this.tete > 4096) {
        this.file = this.file.slice(this.tete);
        this.tete = 0;
      }
    }

    pret() {
      // L'interpréteur peut-il reprendre ?
      const a = this.attente;
      if (!a) return true;
      if (a.t === 'cle') return this.touches.length > 0 || this.commandes.length > 0;
      if (a.t === 'flux') return this.file.length - this.tete <= 64;
      if (a.t === 'temps') return maintenant() >= a.fin || this.touches.length > 0 || this.commandes.length > 0;
      if (a.t === 'pause') return maintenant() >= a.fin;
      return true;
    }

    planifier(delai) {
      if (this.detruite) return;
      if (this.prochain !== null) clearTimeout(this.prochain);
      this.prochain = setTimeout(() => { this.prochain = null; this.tic(); }, Math.max(0, delai));
    }

    tic() {
      this.debiter();
      const debut = maintenant();
      while (this.pret()) {
        const r = this.gen.next();
        if (r.done) {
          if (this.options.surSommaire) this.options.surSommaire(this);
          this.gen = this.interp.basic(this.options.accueil);
          this.attente = null;
          continue;
        }
        this.attente = r.value;
        if (maintenant() - debut > 12) break;
      }
      // Quand reprendre ?
      const a = this.attente;
      let delai = 4;
      if (!this.pret()) {
        if (a.t === 'cle') delai = this.file.length > this.tete ? 16 : -1;
        else if (a.t === 'temps' || a.t === 'pause') delai = Math.min(a.fin - maintenant(), 16);
        else if (a.t === 'flux') delai = this.options.vitesse ? Math.ceil((this.file.length - this.tete - 64) * 10000 / this.options.vitesse) : 0;
      }
      else delai = 0;
      if (delai >= 0) this.planifier(delai);
    }

    reveiller() {
      if (this.prochain === null) this.planifier(0);
    }

    //////////////////////////////////////////////////////////////////////////////
    // Ce que la page peut faire

    focus() {
      if (this.saisie) this.saisie.focus({ preventScroll: true });
      if (this.audio) this.audio.debloquer();
    }

    touche(t) {
      // Frappe une touche : 'ENVOI', 'SUITE'... ou un code (65 pour A)
      const code = codeTouche(t);
      if (code) {
        this.touches.push(code);
        if (this.clavierVirtuel) this.clavierVirtuel.allumer(code);
        this.reveiller();
      }
    }

    taper(texte) {
      // Tape un texte ; \n (retour à la ligne) frappe ENVOI.
      // {SUITE}, {ENVOI}, {ANNULATION}... frappent les touches de fonction.
      const morceaux = String(texte).split(/(\{[A-Za-z\u00c9\u00e9]+\})/);
      for (const m of morceaux) {
        if (/^\{.+\}$/.test(m)) { this.touche(m.slice(1, -1)); continue; }
        for (const ch of m) {
          if (ch === '\n') this.touches.push(ENVOI);
          else if (ch !== '\r' && caractereAccepte(ch)) this.touches.push(ch.codePointAt(0));
        }
      }
      this.reveiller();
    }

    charger(texte) {
      // Charge un programme (comme LOAD) ; un programme en cours est arrêté
      const e = this.interp.charger(String(texte), false);
      this.reveiller();
      if (e) throw new Error('reBASIC\u00a0: ' + e);
      return this;
    }

    ajouter(texte) {
      // Ajoute des lignes au programme (comme MERGE)
      const e = this.interp.charger(String(texte), true);
      this.reveiller();
      if (e) throw new Error('reBASIC\u00a0: ' + e);
      return this;
    }

    executer(ligne, options) {
      // Exécute une ligne comme si on l'avait tapée (« RUN », « LIST »…)
      const echo = !options || options.echo !== false;
      this.commandes.push({ texte: String(ligne), echo });
      this.reveiller();
      return this;
    }

    lancer() {
      // RUN, sans l'afficher
      return this.executer('RUN', { echo: false });
    }

    arreter() {
      // Arrête le programme en cours (comme ANNULATION : « Interruption en ligne n »)
      this.interp.interrompre(false);
      this.reveiller();
      return this;
    }

    programme() { return this.interp.programme().replace(/\r\n/g, '\n'); }

    //////////////////////////////////////////////////////////////////////////////
    // Fichiers de l'ordinateur

    async enregistrerSous(nom) {
      // Enregistre le programme sur l'ordinateur, au même format que SAVE sur la carte SD
      // (« 10 PRINT ... » et fins de ligne CR LF) : le fichier peut être copié sur la carte
      // et relu par LOAD. Renvoie le nom du fichier, ou null si on a annulé.
      nom = nom || 'PROGRAMME.BAS';
      const texte = this.interp.programme();
      this.dernierEnregistrement = 'telechargements';
      if (typeof window.showSaveFilePicker === 'function') {
        try {
          const h = await window.showSaveFilePicker({ suggestedName: nom, types: TYPES_FICHIERS });
          const w = await h.createWritable();
          await w.write(texte);
          await w.close();
          this.dernierEnregistrement = 'choisi';
          return h.name;
        }
        catch (e) {
          if (e && e.name === 'AbortError') return null;
          // Sélecteur refusé (page dans un cadre…) : téléchargement ordinaire
        }
      }
      const a = document.createElement('a');
      a.href = URL.createObjectURL(new Blob([texte], { type: 'text/plain;charset=utf-8' }));
      a.download = nom;
      document.body.appendChild(a);
      a.click();
      a.remove();
      setTimeout(() => URL.revokeObjectURL(a.href), 2000);
      return nom;
    }

    async ouvrirFichier() {
      // Choisit un fichier sur l'ordinateur et le charge (comme LOAD). Une page Vidéotex
      // (.VDT) est copiée sur la carte SD, pour le menu 2.
      // Renvoie { nom, texte }, { nom, page: true } ou null si on a annulé.
      let fichier = null;
      if (typeof window.showOpenFilePicker === 'function') {
        try {
          const [h] = await window.showOpenFilePicker({ types: TYPES_FICHIERS });
          fichier = await h.getFile();
        }
        catch (e) {
          if (e && e.name === 'AbortError') return null;
        }
      }
      if (!fichier) {
        fichier = await new Promise((resoudre) => {
          const i = document.createElement('input');
          i.type = 'file';
          i.accept = '.bas,.BAS,.txt,.TXT,text/plain,.vdt,.VDT';
          i.style.display = 'none';
          i.addEventListener('change', () => { resoudre(i.files[0] || null); i.remove(); });
          i.addEventListener('cancel', () => { resoudre(null); i.remove(); });
          document.body.appendChild(i);
          i.click();
        });
      }
      if (!fichier) return null;
      if (/\.vdt$/i.test(fichier.name)) {
        // Le Vidéotex est un code sur 7 bits : on range les octets tels quels sur la carte
        const octets = new Uint8Array(await fichier.arrayBuffer()).map((b) => b & 0x7F);
        const nom = fichier.name.toUpperCase();
        if (!this.sd.ecrire(nom, octets)) throw new Error('La carte SD est pleine.');
        this.focus();
        return { nom, page: true };
      }
      const texte = await fichier.text();
      this.charger(texte);
      this.focus();
      return { nom: fichier.name, texte };
    }
    enCours() { return this.interp.enCours(); }
    variables() { return this.interp.variables(); }
    texteEcran() { return this.ecran.texte().join('\n'); }

    detruire() {
      this.detruite = true;
      if (this.surTouchePage) document.removeEventListener('keydown', this.surTouchePage);
      if (this.prochain !== null) clearTimeout(this.prochain);
      if (this.audio) this.audio.arreter();
      if (this.racine && this.racine.parentNode) this.racine.parentNode.removeChild(this.racine);
    }
  }

  ////////////////////////////////////////////////////////////////////////////////////
  // Montage automatique : <div data-rebasic> ... </div>
  ////////////////////////////////////////////////////////////////////////////////////

  function lireOptionsHTML(el) {
    const o = {};
    const d = el.dataset;
    const nombre = (v) => (v === undefined ? undefined : Number(v));
    const oui = (v) => (v === undefined ? undefined : v !== 'false' && v !== '0' && v !== 'non');
    if (d.echelle) o.echelle = nombre(d.echelle);
    if (d.vitesse) o.vitesse = nombre(d.vitesse);
    if (d.instructions) o.instructions = nombre(d.instructions);
    if (d.carte) o.carte = d.carte;
    if (d.couleurs !== undefined) o.couleurs = oui(d.couleurs);
    if (d.accueil !== undefined) o.accueil = oui(d.accueil);
    if (d.ok !== undefined) o.afficherOk = oui(d.ok);
    if (d.boutons !== undefined) o.boutons = oui(d.boutons);
    if (d.clavierVirtuel !== undefined) o.clavierVirtuel = oui(d.clavierVirtuel);
    if (d.son !== undefined) o.son = oui(d.son);
    if (d.lancer !== undefined) o.lancer = oui(d.lancer);
    if (d.menu !== undefined) o.menu = oui(d.menu);
    let source = null;
    if (d.programme) {
      const s = document.querySelector(d.programme);
      if (s) source = s.textContent;
    }
    else if (el.textContent.trim()) source = el.textContent;
    if (source !== null) o.programme = source.replace(/^\s*\n/, '');
    return o;
  }

  function monterTout(selecteur) {
    const machines = [];
    for (const el of document.querySelectorAll(selecteur || '[data-rebasic]')) {
      if (el.__rebasic) continue;
      const o = lireOptionsHTML(el);
      el.textContent = '';
      el.__rebasic = new Machine(el, o);
      machines.push(el.__rebasic);
    }
    return machines;
  }

  if (typeof document !== 'undefined') {
    const go = () => monterTout();
    if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', go);
    else setTimeout(go, 0);
  }

  return {
    VERSION,
    creer: (conteneur, options) => new Machine(conteneur, options),
    peutChoisirEmplacement,
    dansUnCadre,
    Machine,
    monterTout,
    touches: Object.assign({}, TOUCHES),
    // Pour les tests et les usages avancés
    _interne: { Ecran, creerInterpreteur, creerCarteSD, ENVOI, SOMMAIRE, ANNULATION, CORRECTION, GUIDE, SUITE, RETOUR }
  };
});
