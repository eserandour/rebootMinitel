10 CLS: X = 5: Y = 5: U = 1: V = 1
20 BOX 0, 0, 79, 68
30 PLOT X, Y, 0
40 X = X + U: Y = Y + V
50 IF X <= 1 OR X >= 78 THEN U = -U
60 IF Y <= 1 OR Y >= 67 THEN V = -V
70 PLOT X, Y
80 GOTO 30
