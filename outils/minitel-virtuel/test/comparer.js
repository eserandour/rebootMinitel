// Compare rebasic.js au simulateur C officiel (outils/simulateur de rebootMinitel) sur
// tous les exemples du tutoriel : mêmes touches, même temps simulé (1 ms par lecture du
// clavier), puis comparaison de l'écran case par case.
// Usage : node test/comparer.js [--detail]   (depuis outils/minitel-virtuel)
// Le simulateur est compilé s'il ne l'est pas encore (outils/simulateur/compiler.sh).
'use strict';
const fs = require('fs');
const path = require('path');
const os = require('os');
const { execFileSync } = require('child_process');
const ReBASIC = require('../rebasic.js');
const { Ecran, creerInterpreteur, creerCarteSD, SOMMAIRE, ANNULATION, CORRECTION, GUIDE, ENVOI, SUITE, RETOUR } = ReBASIC._interne;

const depot = path.resolve(__dirname, '..', '..', '..');
const detail = process.argv.includes('--detail');
const SIM = path.join(depot, 'outils/simulateur/build/simulateur');
if (!fs.existsSync(SIM)) execFileSync('sh', [path.join(depot, 'outils/simulateur/compiler.sh')], { stdio: 'inherit' });
const tuto = fs.readFileSync(path.join(depot, 'outils/tutoriel/tutoriel.md'), 'utf8').split('\n');

// Les exemples de index.html doivent être ceux du dossier exemples/
{
  const h = fs.readFileSync(path.join(__dirname, '..', 'index.html'), 'utf8');
  const d = h.indexOf('const EXEMPLES = {');
  const EXEMPLES = eval('(' + h.slice(d + 'const EXEMPLES = '.length, h.indexOf('\n};', d) + 2) + ')');
  const fichiers = fs.readdirSync(path.join(depot, 'exemples')).filter((f) => /\.BAS$/i.test(f))
    .map((f) => fs.readFileSync(path.join(depot, 'exemples', f), 'utf8').replace(/\r\n/g, '\n').trim()).sort();
  const page = Object.values(EXEMPLES).map((t) => t.trim()).sort();
  if (JSON.stringify(fichiers) !== JSON.stringify(page)) {
    console.log('Les exemples de index.html ne sont pas ceux du dossier exemples/.');
    process.exitCode = 1;
  }
  else console.log(`Les ${page.length} exemples de index.html sont ceux du dossier exemples/.`);
}

function options(s) {
  const o = {};
  const re = /(\w+)(?:=("([^"\\]|\\.)*"|\S+))?/g;
  let m;
  while ((m = re.exec(s))) {
    let v = m[2] === undefined ? true : m[2];
    if (typeof v === 'string' && v.startsWith('"')) v = v.slice(1, -1).replace(/\\"/g, '"');
    o[m[1]] = v;
  }
  return o;
}

function attentes(t) {
  const res = [];
  for (const x of t.match(/\{[^}]*\}|[\s\S]/g) || []) {
    if (x.startsWith('{k:') && !(res.length && res[res.length - 1].startsWith('{WAIT:'))) res.push('{WAIT:60}');
    res.push(x);
  }
  return res.join('');
}

// Les touches du script, comme ajouterTouches() du simulateur (mais en Unicode)
function touches(t) {
  const out = [];
  for (let i = 0; i < t.length;) {
    const c = t[i];
    if (c === '{') {
      const j = t.indexOf('}', i);
      const m = t.slice(i + 1, j);
      i = j + 1;
      const k = { code: 0, toujours: true, special: 0 };
      if (m === 'ANNUL') k.code = ANNULATION;
      else if (m === 'SOMM') k.code = SOMMAIRE;
      else if (m === 'SUITE') k.code = SUITE;
      else if (m === 'RETOUR') k.code = RETOUR;
      else if (m === 'ENVOI') k.code = ENVOI;
      else if (m === 'GUIDE') { k.code = GUIDE; k.toujours = false; }
      else if (m === 'CORR') { k.code = CORRECTION; k.toujours = false; }
      else if (m === 'CLR') k.special = 1;
      else if (m === 'SNAP') k.special = 2;
      else if (m.startsWith('WAIT:')) { k.special = 3; k.code = parseInt(m.slice(5), 10); }
      else if (m.startsWith('k:')) k.code = m.codePointAt(2);
      else continue;
      out.push(k);
      continue;
    }
    if (c === '\r') { i++; continue; }
    if (c === '\n') { out.push({ code: 0x0D, toujours: false, special: 0 }); i++; continue; }
    const cp = t.codePointAt(i);
    out.push({ code: cp, toujours: false, special: 0 });
    i += cp > 0xFFFF ? 2 : 1;
  }
  return out;
}

function executerJS(script, limite) {
  const ecran = new Ecran();
  const liste = touches(script);
  let ms = 0, prochaine = 0, attente = 0, curseur = false, capture = null, interruptions = 0;
  const instantane = () => ecran.cases.map((l) => l.map((x) => Object.assign({}, x)));
  const capturer = (forcer) => { if (capture && !forcer) return; capture = instantane(); };
  const env = {
    carte: 'mega',
    octet(b) { if (b === 0x11) curseur = true; if (b === 0x14) curseur = false; ecran.recevoir(b); },
    cle() {
      ms++;
      while (prochaine < liste.length) {
        const k = liste[prochaine];
        if (k.special === 1) { ecran.effacer(); prochaine++; continue; }
        if (k.special === 2) { capturer(true); prochaine++; continue; }
        if (k.special === 3) {
          if (attente < k.code) { attente++; return 0; }
          attente = 0; prochaine++; continue;
        }
        break;
      }
      if (prochaine < liste.length) {
        const k = liste[prochaine];
        if (k.toujours || curseur) { prochaine++; attente = 0; return k.code; }
      }
      else if (curseur) { capturer(false); return SOMMAIRE; }
      if (++attente > limite) { capturer(false); attente = 0; interruptions++; return ANNULATION; }
      return 0;
    },
    millis: () => ms,
    horloge: () => { const t = 10 * 3600 + 30 * 60 + Math.floor(ms / 1000); return new Date(2026, 10, 14, Math.floor(t / 3600) % 24, Math.floor(t / 60) % 60, t % 60); },
    horlogeOK: true,
    hasard: Math.random,
    son() { }, sonStop() { },
    sd: creerCarteSD('memoire', ''),
    broches: { mode() { }, ecrire() { }, ecrireAnalogique() { }, lire: (b, a) => (a ? Math.trunc(512 + 511 * Math.sin(ms / 2000)) : Math.floor(ms / 3000) % 2) },
    fileOctets: () => 0,
    cadence: () => 0,
    afficherOk: true,
    commandeEnAttente: () => null,
    commandeFaite() { }
  };
  env.sd = sdPartagee;
  const interp = creerInterpreteur(env);
  const gen = interp.basic(true);
  let n = 0;
  while (!gen.next().done) { if (++n > 5e7) throw new Error('boucle'); }
  capturer(false);
  return capture;
}
const sdPartagee = creerCarteSD('memoire', '');

const tmp = fs.mkdtempSync(path.join(os.tmpdir(), 'cmp-'));
const sdC = path.join(tmp, 'sd');
fs.mkdirSync(sdC);

function executerC(script, limite) {
  const ft = path.join(tmp, 't.txt'), fe = path.join(tmp, 'e.txt');
  fs.writeFileSync(ft, script);
  const args = [sdC + '/', ft, fe];
  if (limite) args.push(String(limite));
  execFileSync(SIM, args, { stdio: 'pipe', timeout: 120000 });
  const cases = [];
  for (let i = 0; i < 24; i++) cases.push([]);
  for (const l of fs.readFileSync(fe, 'utf8').split('\n')) {
    const p = l.split('\t');
    if (p.length < 11 || !/^\d/.test(p[0])) continue;
    const v = p.slice(0, 11).map(Number);
    cases[v[0]][v[1]] = { couleur: v[2], inverse: !!v[3], graphique: !!v[4], points: v[5], fond: v[6], souligne: !!v[7], clignote: !!v[8], taille: v[9], couverte: !!v[10], car: p[11] || ' ' };
  }
  return cases;
}

const CHAMPS = ['car', 'couleur', 'fond', 'inverse', 'clignote', 'souligne', 'graphique', 'points', 'taille', 'couverte'];
function texte(cases) {
  return cases.map((l) => l.map((x) => (x.couverte ? ' ' : x.graphique ? (x.points ? '#' : ' ') : x.car)).join('').replace(/\s+$/, '')).join('\n').replace(/\n+$/, '');
}
function differences(a, b) {
  const d = [];
  for (let r = 0; r < 24; r++) for (let c = 0; c < 40; c++) {
    for (const k of CHAMPS) {
      if (a[r][c][k] !== b[r][c][k]) { d.push(`(${r + 1},${c + 1}) ${k}: C=${JSON.stringify(a[r][c][k])} JS=${JSON.stringify(b[r][c][k])}`); break; }
    }
  }
  return d;
}

let n = 0, ok = 0, aleatoires = 0;
const echecs = [];
function comparer(nom, script, limite, aleatoire, code) {
  if (process.env.TRACE) console.error("> " + nom);
  n++;
  const c = executerC(script, limite);
  const j = executerJS(script, limite || 400000);
  const d = differences(c, j);
  if (d.length === 0) { ok++; return; }
  if (aleatoire) { aleatoires++; return; }
  echecs.push(nom);
  console.log(`\n=== ${nom} : ${d.length} case(s) différente(s)`);
  console.log(code.join('\n'));
  console.log(d.slice(0, 8).join('\n'));
  if (detail) {
    console.log('--- C\n' + texte(c));
    console.log('--- JS\n' + texte(j));
  }
}

// L'écran d'accueil
comparer('accueil', '', null, false, ['(accueil)']);

for (let i = 0; i < tuto.length; i++) {
  if (tuto[i].trim() !== '```basic') continue;
  let j = i + 1;
  while (tuto[j].trim() !== '```') j++;
  const code = tuto.slice(i + 1, j);
  const m = (tuto[j + 1] || '').match(/^<!--(ecran|test)\s*(.*?)-->\s*$/);
  const o = m ? options(m[2]) : {};
  const genre = m ? m[1] : 'test';
  let k = typeof o.keys === 'string' ? o.keys.replace(/\\n/g, '\n') : '';
  k = attentes(k);
  let script;
  if (o.mode === 'direct') script = '{CLR}' + code.join('\n') + '\n' + k;
  else script = 'NEW\n' + code.join('\n') + '\n' + (genre === 'ecran' ? '{CLR}' : '') + 'RUN\n' + k;
  const aleatoire = /RND|TIMER/.test(code.join('\n'));
  comparer('ligne ' + (i + 1), script, o.limit ? parseInt(o.limit, 10) : null, aleatoire, code);
  i = j;
}

// Les exemples du simulateur et quelques cas en plus
const extra = [
  ['erreurs', 'PRINT 2+*3\nPRINT 1/0\n10 GOSUB 99\nRUN\nRETURN\nNEXT\nX = "A"\nCOLOR 9\nLOCATE 1,25\nPRINTA\nSIZE 3:PRINT "AB"\n'],
  ['nombres', 'PRINT 1/3, 2/3, 1E9, 1.5E-5\nPRINT 123456789, 0.1+0.2, -7.25, 3E38*10\nPRINT 100000/7, 1E-4, 99999999\nPRINT SQR(2)*SQR(2), INT(-2.5), 5 MOD 3, -5 MOD 3\nPRINT NOT 0, NOT -1, 3 AND 5, 3 OR 5, 2^0.5\nPRINT STR$(1/7)+"|", VAL(" -12.5E2x"), HEX$(-1), HEX$(65535)\n'],
  ['chaines', 'A$="éèàçù":PRINT LEN(A$), ASC(A$), MID$(A$,2,2), RIGHT$(A$,2)\nB$="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ":PRINT B$:PRINT LEN(B$)\nC$="éééééééééééééééééé":PRINT LEN(C$)\nPRINT INSTR(3,"ABCABC","BC"), UPPER$("abcé"), LOWER$("ABC")\nPRINT "A" < "B", "a" > "Z", "AB" = "AB"\n'],
  ['edit', '10 PRINT "Bonjour Paris"\nEDIT 10\n{CORR}{CORR}{CORR}{CORR}{CORR}{CORR}Lyon"\nLIST\nRUN\n'],
  ['renum', '1 PRINT "A":GOTO 3\n2 ON X GOSUB 1,3\n3 IF A THEN 1 ELSE 2\n4 RESTORE 2\nRENUM 100,5\nLIST\n'],
  ['aide', '{GUIDE}{SUITE}{SUITE}{SUITE}{SUITE}{RETOUR}{SNAP}{ENVOI}'],
  ['aide2', '{GUIDE}{SUITE}{SUITE}{SUITE}{ENVOI}PRINT "ok"\n'],
  ['taille', 'CLS:LOCATE 1,1:SIZE 1:PRINT "R1":SIZE 0\nLOCATE 3,5:SIZE 3:INVERSE 1:PRINT "XY";:SIZE 2:PRINT "Z":SIZE 0:INVERSE 0\nCOLOR 3:PAPER 4:UNDERLINE 1:PRINT "A BC":UNDERLINE 0:PAPER 0\nPLOT 10,10:INVERSE 1:SIZE 2:PRINT "Q":PLOT 11,11\n'],
  ['corr', 'PRINT "abc{CORR}{CORR}d"\nSIZE 2:PRINT "ABCDEFGHIJKLMNOPQRSTUV{CORR}{CORR}{CORR}"\n'],
  ['defile', 'FOR I=1 TO 30:PRINT I:NEXT\n'],
  ['fichiers', 'NEW\n10 OPEN "T" FOR OUTPUT AS #1\n20 PRINT #1, "a,b", 12; "x"\n25 PRINT #1, "é", TAB(5); 3\n30 CLOSE\n40 OPEN "T" FOR INPUT AS #1\n50 INPUT #1, A$, B$, C$\n60 LINE INPUT #1, D$\n70 PRINT A$;"|";B$;"|";C$;"|";D$;"|";EOF(1)\nRUN\nSAVE "PROG"\nNEW\nLOAD "PROG"\nLIST 10-30\nFILES\nKILL "PROG"\nLOAD "PROG"\n'],
  ['cercles', 'CLS:CIRCLE 40,36,30:CIRCLE 20,20,8,1,1:BOX 60,50,79,71,1,1:LINE 0,71,79,0:BOX 61,51,70,60,0,1:CIRCLE 40,0,50,1,1\nGOTO 10\n'],
  ['while', '10 I=0:WHILE I<3:J=0:WHILE J<2:PRINT I;J;" ";:J=J+1:WEND:I=I+1:WEND\n20 WHILE 0:PRINT "non":WEND:PRINT "fin"\n30 FOR I=1 TO 3:FOR I=1 TO 2:NEXT I:NEXT\nRUN\n'],
  ['imbric', 'PRINT ((((((((1))))))))\nPRINT ABS(ABS(ABS(ABS(ABS(ABS(-1))))))\n10 GOSUB 10\nRUN\n'],
  ['data', '10 READ A,B$,C:PRINT A;B$;C\n20 DATA 1, deux , "trois": DATA 3\n30 READ D\nRUN\n'],
  ['memoire', 'DIM A(60):DIM B(5)\nDIM C(2)\nA(61)=1\n']
];
for (const [nom, s] of extra) comparer(nom, s.startsWith('{') || s.startsWith('NEW') ? s : 'NEW\n' + s, null, false, [s]);

console.log(`\n${n} écrans comparés : ${ok} identiques, ${aleatoires} différents à cause du hasard, ${echecs.length} différences.`);
fs.rmSync(tmp, { recursive: true, force: true });
process.exit(echecs.length || process.exitCode ? 1 : 0);
