#include <stdio.h>
#include <stdlib.h> // Para EXIT_FAILURE
#include <string.h> // Para strrchr, strcmp, snprintf

#define MAX_LINHA 1024        // Para buffer de linha
#define MAX_NOME_ARQUIVO 1024 // Para nomes de arquivo
#define MAX_MNEMONIC 32
#define UPCODEFILE "upcode.txt"
#define MAX_TAMANHO_SIMBOLO 50
#define MAX_TAMANHO_MNEMONIC 50
#define MAX_PENDENCIAS 50
#define MAX_CODIGO_OBJ 128
#define MAX_ARGS_OPR 2
#define BUFFER_SIZE 1024

typedef struct
{
    char mnemonic[MAX_TAMANHO_MNEMONIC]; // Nome do símbolo, ex: "ADD"
    int opcode;                          // Código numérico, ex: 1
    int size;                            // Tamanho em palavras, ex: 2
} Mnemonic;

typedef struct
{
    char simbolo[MAX_TAMANHO_SIMBOLO];
    int valor;
    int def;
    int pendencias[MAX_PENDENCIAS];
    int num_pendencias;
} TabelaSimbolo;

typedef struct
{
    int endereco;
    int opcode;
    int args[MAX_ARGS_OPR];
    int line;
} CodigoObj;

int insertLineCodigoObj(CodigoObj *codigo, int endereco, int opcode, int *args, int *line, int maxLines)
{
    if (*line >= maxLines)
    {
        return 0; // Tabela cheia
    }
    codigo[*line].endereco = endereco;
    codigo[*line].opcode = opcode;
    codigo[*line].line = *line;
    for (int j = 0; j < MAX_ARGS_OPR; j++)
    {
        codigo[*line].args[j] = args[j];
    }

    (*line)++;
    return 1; // Sucesso
}

int findLineCodigoObjByAdress(CodigoObj *codigo, int maxLines, int endereco)
{
    for (int i = 0; i < maxLines; i++)
    {
        int currentAdress = codigo[i].endereco;
        if (currentAdress == endereco)
        {
            return i; // Encontrou
        }
        if (i != (maxLines - 1))
        {
            int adressOfStartNextLine = codigo[i + 1].endereco;

            if (endereco > currentAdress && endereco < adressOfStartNextLine)
            {
                return i;
            }
        }
    }
    return -1; // Não encontrado
}

int updateArgByAdress(CodigoObj *codigo, int maxLines, int endereco, int novo_arg)
{
    int idx = findLineCodigoObjByAdress(codigo, maxLines, endereco);
    if (idx >= 0)
    {
        int lineAdress = codigo[idx].endereco;
        int idxArg = endereco - lineAdress - 1;
        codigo[idx].args[idxArg] = novo_arg;
        return 1; // Atualizado
    }
    return 0; // Não encontrou para atualizar
}

char *printCodObj(CodigoObj *codigo, int maxLines)
{
    // Buffer para armazenar a string final
    static char finalCod[BUFFER_SIZE];
    finalCod[0] = '\0'; // Limpa o buffer

    char temp[32];
    for (int l = 0; l < maxLines; l++)
    {
        // Adiciona opcode
        snprintf(temp, sizeof(temp), "%d ", codigo[l].opcode);
        strcat(finalCod, temp);

        if (codigo[l].opcode == 9)
        {
            // Adiciona argumentos válidos
            for (int a = 0; a < MAX_ARGS_OPR; a++)
            {
                int arg = codigo[l].args[a];
                snprintf(temp, sizeof(temp), "%d ", arg);
                strcat(finalCod, temp);
            }
        }
        else
        {
            snprintf(temp, sizeof(temp), "%d ", codigo[l].args[0]);
            strcat(finalCod, temp);
        }
    }

    // Remove espaço extra final, se existir
    int len = strlen(finalCod);
    if (len > 0 && finalCod[len - 1] == ' ')
        finalCod[len - 1] = '\0';

    return finalCod;
}

char *printListaPendencias(TabelaSimbolo *table, int maxLines)
{
    // Buffer para armazenar a string final
    static char list[BUFFER_SIZE];
    list[0] = '\0'; // Limpa o buffer

    char temp[32];
    snprintf(temp, sizeof(temp), "LISTA DE PENDENCIAS:\n");
    strcat(list, temp);

    for (int l = 0; l < maxLines; l++)
    {
        snprintf(temp, sizeof(temp), "Simb: %s\nValor: %d\nDef: %d\n", table[l].simbolo, table[l].valor, table[l].def);
        strcat(list, temp);

        // Buffer para armazenar a string da lista desse simbolo em especifico
        static char valuesList[MAX_PENDENCIAS];
        valuesList[0] = '\0'; // Limpa o buffer
        char tempValues[32];

        snprintf(tempValues, sizeof(tempValues), "Pendencias: ");
        strcat(valuesList, tempValues);

        for (int p = 0; p < MAX_PENDENCIAS; p++)
        {
            snprintf(tempValues, sizeof(tempValues), "%d ", table[l].pendencias[p]);
            strcat(valuesList, tempValues);
        }

        // Remove espaço extra final, se existir
        int len = strlen(valuesList);
        valuesList[len - 1] = '\n';

        snprintf(temp, sizeof(temp), "%c", valuesList);
        strcat(list, temp);
    }

    // Remove espaço extra final, se existir
    int len = strlen(list);
    if (len > 0 && list[len - 1] == ' ')
    {
        list[len - 1] = '\0';
    }

    return list;
}

int findSimboloTabela(TabelaSimbolo tabela[], int n, const char *nome)
{
    for (int i = 0; i < n; i++)
    {
        if (strcmp(tabela[i].simbolo, nome) == 0)
        {
            return i; // Retorna índice se encontrado
        }
    }
    return -1; // Não encontrado
}

int insertSimboloTabela(TabelaSimbolo tabela[], int *n, int maxTable, const char *nome, int valor, int def)
{
    if (*n >= maxTable)
    {
        return 0; // Tabela cheia
    }
    strcpy(tabela[*n].simbolo, nome);
    tabela[*n].valor = valor;
    tabela[*n].def = def;
    tabela[*n].num_pendencias = 0;
    (*n)++;
    return 1; // Sucesso
}

int insertPendenciaAtSimbolo(TabelaSimbolo *simbolo, int desloc)
{
    if (simbolo->num_pendencias >= MAX_PENDENCIAS)
        return 0; // Lista cheia
    simbolo->pendencias[simbolo->num_pendencias] = desloc;
    simbolo->num_pendencias++;
    return 1; // Sucesso
}

int loadMnemonics(const char *filename, Mnemonic *table, int max_size)
{
    FILE *f = fopen(filename, "r");
    if (!f)
        return -1;

    char line[64];
    int count = 0;
    while (fgets(line, sizeof(line), f) && count < max_size)
    {
        char mnemonic[8];
        int opcode, size;
        // Lê formato: ADD,01,2
        if (sscanf(line, "%7[^,],%d,%d", mnemonic, &opcode, &size) == 3)
        {
            strcpy(table[count].mnemonic, mnemonic);
            table[count].opcode = opcode;
            table[count].size = size;
            count++;
        }
    }
    fclose(f);
    return count;
}

int findMnemonic(const char *mnemonic, Mnemonic *table, int num)
{
    for (int i = 0; i < num; i++)
    {
        if (strcmp(table[i].mnemonic, mnemonic) == 0)
        {
            return i; // Encontrou: retorna índice
        }
    }
    return -1; // Não encontrou
}

void separador(char *str_original, char *words[1024])
{
    // recebe o ponteiro com o início da string a ser processada e uma array, e modifica a array para conter todas as palavras encontradas
    char *str = str_original;
    int word_count = 0;

    const char DELIMITER = ' ';

    char *p = str; // 'p' fica no início da string
    char *e;       // 'e' fica no final efetivo da string
    char *q;       // 'q' vai iterando por toda a string e marca o começo de uma palavra
    char *r;       // 'r' itera a partir do início da palavra em q até o fim da palavra

    // colocar ponteiro 'e' no final efetivo da string
    for (e = p; *e != '\0'; e++)
    {
        if (*e == ';')
            break;
        // Loop basicamente vazio apenas para avançar 'e' até o fim ou até encontrar ';'
    }

    q = p;

    while (q < e)
    {
        // procura o primeiro caractere interessante (não espaço branco) e coloca 'q' nele
        while (q < e && *q == DELIMITER)
        {
            q++;
        }

        // condição de parada, se 'q' chegar ao final efetivo da string termina a função
        if (q == e)
        {
            break;
        }

        r = q;
        // procura o primeiro caractere interessante (espaço branco) e coloca 'r' nele
        while (r < e && *r != DELIMITER)
        {
            r++;
        }

        // cria uma nova string e copia a palavra entre 'q' e 'r' nela, coloca essa nova palavra no array recebido
        long tamanho = r - q;
        char *new_word = malloc(tamanho + 1);
        for (long i = 0; i < tamanho; i++)
        {
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

int main(int argc, char *argv[])
{
    FILE *arquivoEntrada;
    FILE *arquivoSaidaO1;
    FILE *arquivoSaidaO2;
    char linha[MAX_LINHA];

    // Verificar qtd de argumentos
    if (argc != 2)
    {
        fprintf(stderr, "Erro com quantidade de argumentos recebidos");
        return EXIT_FAILURE;
    }

    const char *nomeArquivoEntrada = argv[1];
    char nomeArquivoSaidaO1[MAX_NOME_ARQUIVO];
    char nomeArquivoSaidaO2[MAX_NOME_ARQUIVO];
    Mnemonic tableMnemonic[MAX_MNEMONIC];
    int quantMnemonic = loadMnemonics(UPCODEFILE, tableMnemonic, MAX_MNEMONIC);

    // Tentar encontrar a última ocorrência de '.'
    const char *extensao = strrchr(nomeArquivoEntrada, '.'); // PROVAVELMENTE NÃO É PERMITIDO POR QUE PRECISA DA LIB <string.h>, SUBSTITUIR POR IMPLEMENTAÇÃO MANUAL
    if (extensao != NULL)
    {
        // Tamanho da parte antes da extensão
        int tamBase = extensao - nomeArquivoEntrada;
        // Copia a primeira parte do nome no nome dos arquivos de saída
        // "%.*s" copia um número específico (tamBase) de caracteres da string
        snprintf(nomeArquivoSaidaO1, MAX_NOME_ARQUIVO, "%.*s.o1", tamBase, nomeArquivoEntrada);
        snprintf(nomeArquivoSaidaO2, MAX_NOME_ARQUIVO, "%.*s.o2", tamBase, nomeArquivoEntrada);
    }
    else
    {
        // Só coloca ".o1" ou ".o2" no final do nome do arquivo
        snprintf(nomeArquivoSaidaO1, MAX_NOME_ARQUIVO, "%s.o1", nomeArquivoEntrada);
        snprintf(nomeArquivoSaidaO2, MAX_NOME_ARQUIVO, "%s.o2", nomeArquivoEntrada);
    }

    // Carregando arquivos
    arquivoEntrada = fopen(nomeArquivoEntrada, "r");
    if (arquivoEntrada == NULL)
    {
        perror("Erro ao abrir o arquivo de entrada");
        return EXIT_FAILURE;
    }
    arquivoSaidaO1 = fopen(nomeArquivoSaidaO1, "w");
    if (arquivoSaidaO1 == NULL)
    {
        perror("Erro ao abrir o arquivo de saída");
        fclose(arquivoEntrada);
        return EXIT_FAILURE;
    }
    arquivoSaidaO2 = fopen(nomeArquivoSaidaO2, "w");
    if (arquivoSaidaO2 == NULL)
    {
        perror("Erro ao abrir o arquivo de saída");
        fclose(arquivoEntrada);
        return EXIT_FAILURE;
    }

    printf("--- Conteúdo de %s (impresso no console) ---\n", nomeArquivoEntrada);

    // Ler o arquivo de entrada linha por linha
    while (fgets(linha, sizeof(linha), arquivoEntrada) != NULL)
    {
        // Imprime o conteúdo da linha no console
        printf("%s", linha);

        char *p, *q, *r, *e;
        /* 'p' should point to the start of the buffer, not write into *p */
        p = linha;
        /* set 'e' to the effective end (either '\0' or ';') */
        for (e = p; *e != '\0' && *e != ';'; e++)
        {
            /* advance until end or ';' */
        }

        /* skip leading spaces */
        q = p;
        while (q < e && *q == ' ')
        {
            q++;
        }

        /* r marks the end of the first token (or next space) */
        r = q;
        while (r < e && *r != ' ')
        {
            r++;
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