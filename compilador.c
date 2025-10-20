#include <stdio.h>
#include <stdlib.h> // Para EXIT_FAILURE
#include <string.h> // Para strrchr, strcmp, snprintf

#define MAX_LINHA 1024 // Para buffer de linha
#define MAX_NOME_ARQUIVO 1024 // Para nomes de arquivo

int main(int argc, char *argv[]) {
    FILE *arquivoEntrada;
    FILE *arquivoSaidaO1;
    FILE *arquivoSaidaO2;
    char linha[MAX_LINHA];

    // Verificar qtd de argumentos
    if (argc != 2) {
        fprintf(stderr, "Erro com quantidade de argumentos recebidos");
        return EXIT_FAILURE; 
    }

    const char *nomeArquivoEntrada = argv[1];
    char nomeArquivoSaidaO1[MAX_NOME_ARQUIVO];
    char nomeArquivoSaidaO2[MAX_NOME_ARQUIVO];

    // Tentar encontrar a última ocorrência de '.'
    const char *extensao = strrchr(nomeArquivoEntrada, '.');
    if (extensao != NULL) {
        // Tamanho da parte antes da extensão
        int tamBase = extensao - nomeArquivoEntrada;
        // Copia a primeira parte do nome no nome dos arquivos de saída
        // "%.*s" copia um número específico (tamBase) de caracteres da string
        snprintf(nomeArquivoSaidaO1, MAX_NOME_ARQUIVO, "%.*s.o1", tamBase, nomeArquivoEntrada);
        snprintf(nomeArquivoSaidaO2, MAX_NOME_ARQUIVO, "%.*s.o2", tamBase, nomeArquivoEntrada);
    } else {
        // Só coloca ".o1" ou ".o2" no final do nome do arquivo
        snprintf(nomeArquivoSaidaO1, MAX_NOME_ARQUIVO, "%s.o1", nomeArquivoEntrada);
        snprintf(nomeArquivoSaidaO2, MAX_NOME_ARQUIVO, "%s.o2", nomeArquivoEntrada);
    }

    // Carregando arquivos
    arquivoEntrada = fopen(nomeArquivoEntrada, "r");
    if (arquivoEntrada == NULL) {
        perror("Erro ao abrir o arquivo de entrada");
        return EXIT_FAILURE;
    }
    arquivoSaidaO1 = fopen(nomeArquivoSaidaO1, "w");
    if (arquivoSaidaO1 == NULL) {
        perror("Erro ao abrir o arquivo de saída");
        fclose(arquivoEntrada); 
        return EXIT_FAILURE;
    }
    arquivoSaidaO2 = fopen(nomeArquivoSaidaO2, "w");
    if (arquivoSaidaO2 == NULL) {
        perror("Erro ao abrir o arquivo de saída");
        fclose(arquivoEntrada); 
        return EXIT_FAILURE;
    }

    printf("--- Conteúdo de %s (impresso no console) ---\n", nomeArquivoEntrada);

    // Ler o arquivo de entrada linha por linha
    while (fgets(linha, sizeof(linha), arquivoEntrada) != NULL) {
        // Imprime o conteúdo da linha no console
        printf("%s", linha);

        // Escreve o conteúdo da linha no arquivo de saída
        fputs(linha, arquivoSaidaO1);
        fputs(linha, arquivoSaidaO2);
    }

    printf("------------------------------------------------\n");
    // Mensagem de sucesso atualizada
    printf("Arquivo lido e copiado de %s para %s com sucesso!\n", 
           nomeArquivoEntrada, nomeArquivoSaidaO1);
    printf("Arquivo lido e copiado de %s para %s com sucesso!\n", 
           nomeArquivoEntrada, nomeArquivoSaidaO2);

    // Fecha os arquivos
    fclose(arquivoEntrada);
    fclose(arquivoSaidaO1);
    fclose(arquivoSaidaO2);

    return 0; 
}