10 CLS: RESTORE: N = RND(5)
20 FOR I = 1 TO N: READ M$: NEXT
30 L = LEN(M$): T$ = "": E = 0
40 FOR I = 1 TO L: T$ = T$ + "-": NEXT
50 PRINT: PRINT T$; "   erreurs : "; E; "/7"
60 IF T$ = M$ THEN PRINT "Gagné !": END
70 IF E = 7 THEN PRINT "Perdu, c'était "; M$: END
80 INPUT "Lettre"; C$: C$ = UPPER$(LEFT$(C$, 1))
90 IF INSTR(M$, C$) = 0 THEN E = E + 1: GOTO 50
100 FOR I = 1 TO L
110 IF MID$(M$, I, 1) = C$ THEN GOSUB 200
120 NEXT: GOTO 50
200 T$ = LEFT$(T$, I - 1) + C$ + MID$(T$, I + 1): RETURN
300 DATA MINITEL, ARDUINO, CLAVIER, MODEM, ECRAN
