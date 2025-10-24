; Teste 8: Máximo permitido pela especificação
MAX_MACRO1: MACRO &ARG1, &ARG2
            LOAD &ARG1
            ADD &ARG2
            STORE TEMP1
            ENDMACRO

MAX_MACRO2: MACRO &PAR1, &PAR2
            LOAD &PAR1
            SUB &PAR2
            STORE TEMP2
            ENDMACRO

TESTE: INPUT NUM1
       INPUT NUM2
       MAX_MACRO1 NUM1, NUM2
       MAX_MACRO2 NUM1, NUM2
       OUTPUT TEMP1
       OUTPUT TEMP2
       STOP

NUM1: SPACE
NUM2: SPACE
TEMP1: SPACE
TEMP2: SPACE

