#include <stdio.h>
#include <stdlib.h> // Para EXIT_FAILURE
#include <string.h> // Para strrchr, strcmp, snprintf

#define MAX_LINHA 1024 // Para buffer de linha
#define MAX_NOME_ARQUIVO 1024 // Para nomes de arquivo

void separador(char *str_original, char *words[1024]) {
    // recebe o ponteiro com o início da string a ser processada e uma array, e modifica a array para conter todas as palavras encontradas
    char *str = str_original;
    int word_count = 0;
    
    const char DELIMITER = ' ';

    char *p = str;      // 'p' fica no início da string
    char *e;            // 'e' fica no final efetivo da string
    char *q;            // 'q' vai iterando por toda a string e marca o começo de uma palavra
    char *r;            // 'r' itera a partir do início da palavra em q até o fim da palavra

    // colocar ponteiro 'e' no final efetivo da string 
    for (e = p; *e != '\0'; e++) {
        if (*e == ';') break;
        // Loop basicamente vazio apenas para avançar 'e' até o fim ou até encontrar ';'
    }
    
    q = p;

    while (q < e) {
        // procura o primeiro caractere interessante (não espaço branco) e coloca 'q' nele
        while (q < e && *q == DELIMITER) {
            q++;
        }

        // condição de parada, se 'q' chegar ao final efetivo da string termina a função
        if (q == e) {
            break; 
        }
        
        r = q;
        // procura o primeiro caractere interessante (espaço branco) e coloca 'r' nele
        while (r < e && *r != DELIMITER) {
            r++;
        }

        // cria uma nova string e copia a palavra entre 'q' e 'r' nela, coloca essa nova palavra no array recebido
        long tamanho = r - q;
        char* new_word = malloc(tamanho + 1);
        for(long i = 0; i < tamanho; i++){
            new_word[i] = q[i];
        }
        new_word[tamanho] = '\0';
        words[word_count] = new_word;
        word_count++;

        // coloca 'q' depois da nova palavra e repete o processo
        q = r;
    }
    
    // código de teste da funcionalidade da função
    // for(int i = 0; i < word_count; i++){
    //     long tamanho = words[i+1] - words[i];
    //     printf("Palavra encontrada: '");
    //     for (long j = 0; words[i][j] != '\0'; j++) {
    //         putchar(words[i][j]);
    //     }
    //     printf("'\n");
    // }
}

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
    const char *extensao = strrchr(nomeArquivoEntrada, '.'); // PROVAVELMENTE NÃO É PERMITIDO POR QUE PRECISA DA LIB <string.h>, SUBSTITUIR POR IMPLEMENTAÇÃO MANUAL
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

        char *p, *q, *r, *e;
        *p = linha;
        e = p;
        for (e; (*e == "\0" || *e == ";"); e++);
        q = p;
        for (size_t i = 0; i < sizeof(linha); i++)
        {
            if (*q != " ")
            {
                break;
            }
            q++;
        }
        r = q;
        for (size_t i = 0; i < sizeof(linha); i++)
        {
            if (*q != " ")
            {
                break;
            }
            q++;
        }
        

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