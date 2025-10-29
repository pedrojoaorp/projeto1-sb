#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/structs.h"
#include "../include/preprocessador.h"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Uso: %s arquivo.asm\n", argv[0]);
        return 1;
    }

    char output_filename[512];
    strcpy(output_filename, argv[1]);
    char *dot = strrchr(output_filename, '.');

    if (dot)
        strcpy(dot, ".pre");
    else
        strcat(output_filename, ".pre");

    int result = preprocess(argv[1], output_filename);

    if (result == 0)
    {
        printf("Arquivo .pre gerado: %s\n", output_filename);
    }
    else
    {
        printf("Erro durante o pre-processamento.\n");
    }

    return result;
}
