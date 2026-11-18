10 P = 1
20 CLS: PRINT "Page "; P; " sur 3"
30 PRINT: PRINT "SUITE : page suivante"
40 PRINT "RETOUR : page précédente"
50 PRINT "ENVOI : fin"
60 K = KEY: IF K = 0 THEN 60
70 IF K = 4936 AND P < 3 THEN P = P + 1: GOTO 20
80 IF K = 4930 AND P > 1 THEN P = P - 1: GOTO 20
90 IF K <> 4929 THEN 60
100 CLS: PRINT "Au revoir"
