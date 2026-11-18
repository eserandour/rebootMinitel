10 CLS: PRINT "J'ai choisi un nombre"
15 PRINT "entre 1 et 100."
20 M = RND(100): C = 0
30 INPUT "Ta proposition"; P
40 C = C + 1
50 IF P < M THEN PRINT "C'est plus.": GOTO 30
60 IF P > M THEN PRINT "C'est moins.": GOTO 30
70 PRINT "Gagné en "; C; " coups !"
