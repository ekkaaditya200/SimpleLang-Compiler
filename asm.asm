 MVI A, 10
  STA a
  MVI A, 20
  STA b
  MOV A, a
  ADD b
  STA c
  MOV A, c
  CPI 30
  JNZ LABEL0
  MOV A, c
  ADI 100
  STA c
LABEL0: