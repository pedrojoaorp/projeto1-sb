TROCA_VALORES: MACRO &X, &Y
               COPY &X, AUX
               COPY &Y, &X
               COPY AUX, &Y
               ENDMACRO

NORMALIZA: MACRO &VAL
           LOAD &VAL
           ADD OFFSET
           STORE &VAL
           ENDMACRO

INICIO: INPUT A
        INPUT B
        TROCA_VALORES A, B
        NORMALIZA A
        NORMALIZA B
        OUTPUT A
        OUTPUT B
        STOP

A: SPACE
B: SPACE
AUX: SPACE
OFFSET: SPACE 10

; Teste 7: Testando remoção de & nos parâmetros
