    INPUT N 
    LOAD N 
FAT: SUB ONE 
    JMPZ FIM 
    STORE AUX 
    MULT N 
    STORE N 
    LOAD AUX 
    JMP FAT 
FIM: Print N ;instrução inexistente
    STOP
AUX: SPACE 3
N: SPACE
ONE: CONST 1

COPY A, B 
ROT: COPY C, D
ULTIMA: COPY E, F