MULTIPLICA: MACRO &NUM, &FATOR
            LOAD &NUM
            MUL &FATOR
            STORE &NUM
            ENDMACRO

CALC_AREA: MACRO &BASE, &ALTURA
           MULTIPLICA &BASE, &ALTURA
           ENDMACRO

PROGRAMA: INPUT BASE
          INPUT ALTURA
          CALC_AREA BASE, ALTURA
          OUTPUT BASE
          STOP

BASE: SPACE
ALTURA: SPACE

; Teste 6: Operações mais elaboradas
