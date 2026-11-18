10 CLS: X = 40: Y = 36: U = 1: V = 0: N = 0
20 R = RND(8)
30 IF R = 1 THEN W = U: U = V: V = -W
40 IF R = 2 THEN W = U: U = -V: V = W
50 X = X + U: Y = Y + V
60 IF X < 0 OR X > 79 OR Y < 0 OR Y > 70 THEN 90
70 IF POINT(X, Y) THEN 90
80 PLOT X, Y: N = N + 1: GOTO 20
90 LOCATE 1, 1: PRINT N; " pas.";
100 GOTO 100
