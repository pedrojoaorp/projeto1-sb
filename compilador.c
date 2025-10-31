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
#define MAX_TAMANHO_MEMORIA 1024
#define MAX_QTD_SIMBOLOS 1024

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
    int memoria[MAX_TAMANHO_MEMORIA];
    int tamanho;
} CodigoObj;

int insertInCodigoObj(CodigoObj *codigo, int dado)
{
    if (codigo->tamanho >= MAX_TAMANHO_MEMORIA)
    {
        return 0; // Tabela cheia
    }
    codigo->memoria[codigo->tamanho] = dado;
    (codigo->tamanho)++;
    return 1; // Sucesso
}

int findByAddressInCodigoObj(CodigoObj *codigo, int endereco)
{
    if (endereco >= codigo->tamanho)
    {
        return -1; // Não encontrado
    }
    return codigo->memoria[endereco];
}

int updateByAddressInCodigoObj(CodigoObj *codigo, int endereco, int novo_dado)
{
    if (endereco >= codigo->tamanho)
    {
        return -1; // Não encontrado
    }
    int velho_dado;
    velho_dado = codigo->memoria[endereco];
    codigo->memoria[endereco] = novo_dado;
    return velho_dado;
}

int addToAddressInCodigoObj(CodigoObj *codigo, int endereco, int novo_dado)
{
    if (endereco >= codigo->tamanho)
    {
        return -1; // Não encontrado
    }
    codigo->memoria[endereco] += novo_dado;
    return codigo->memoria[endereco];
}

char *printCodObj(CodigoObj *codigo)
{
    int buffer_size = 0;
    for (size_t i = 0; i < codigo->tamanho; i++)
    {
        const char *format = (i == 0) ? "%d" : " %d";
        buffer_size += snprintf(NULL, 0, format, codigo->memoria[i]);
    }
    buffer_size++;

    char *finalCod = (char *)malloc(buffer_size);
    if (finalCod == NULL)
    {
        perror("erro para alocar memória em printCodObj");
        return NULL;
    }

    char *current_pos = finalCod;
    int remaining_space = buffer_size;

    for (size_t i = 0; i < codigo->tamanho; i++)
    {
        const char *format = (i == 0) ? "%d" : " %d";
        int chars_added = snprintf(current_pos, remaining_space, format, codigo->memoria[i]);
        current_pos += chars_added;
        remaining_space -= chars_added;
    }
    printf("%s\n", finalCod);
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
    return ((*n)-1); // Sucesso, retorna o ponteiro para o símbolo recém criado
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

int separador(char *str_original, char *words[1024])
{
    char *str = str_original;
    int word_count = 0;

    const char DELIMITER = ' ';

    char *p = str;
    char *e;
    char *q;
    char *r;

    // Definir 'e' no final efetivo da string
    for (e = p; *e != '\0'; e++)
    {
        if (*e == ';' || *e == '\n')
            break;
    }

    q = p;

    while (q < e)
    {
        // Pula espaços e virgulas
        while (q < e && (*q == DELIMITER || *q == ','))
        {
            q++;
        }

        if (q == e)
        {
            break;
        }

        r = q;
        // procura o fim do token (espaço, virgula, ou fim)
        while (r < e && *r != DELIMITER && *r != ',')
        {
            r++;
        }

        long tamanho = r - q;
        if (tamanho > 0)
        {
            char *new_word = malloc(tamanho + 1);
            for (long i = 0; i < tamanho; i++)
            {
                new_word[i] = q[i];
            }
            new_word[tamanho] = '\0';
            words[word_count] = new_word;
            word_count++;
        }

        q = r;
    }

    return word_count;
}

// código de teste da funcionalidade da função
// for (int i = 0; i < word_count; i++)
// {
//     long tamanho = words[i + 1] - words[i];
//     printf("Palavra encontrada: '");
//     for (long j = 0; words[i][j] != '\0'; j++)
//     {
//         putchar(words[i][j]);
//     }
//     printf("'\n");

void compileFile(FILE *arquivoEntrada, FILE *arquivoSaidaO1, FILE *arquivoSaidaO2)
{
    char linha[1024];   // buffer para a linha lida
    char *tokens[1024]; // array para armazenar os tokens da linha
    int word_count;

    CodigoObj codigo;
    TabelaSimbolo tabelaSimbolos[MAX_QTD_SIMBOLOS];
    int qtd_simbolos = 0;
    Mnemonic mnemonicTable[MAX_MNEMONIC];
    int qtd_mnemonicos = loadMnemonics("upcode.txt", &mnemonicTable, MAX_MNEMONIC);

    // Ler o arquivo de entrada linha por linha
    while (fgets(linha, sizeof(linha), arquivoEntrada) != NULL)
    {
        // printf("%s", linha);
        //  Obter tokens da linha, usando separador
        word_count = separador(linha, tokens);

        char *label = NULL;
        char *opr = NULL;
        char *arg1 = NULL;
        char *arg2 = NULL;

        if (word_count > 0)
        {
            // Se primeiro token terminar com ':' é label
            int len = strlen(tokens[0]);
            if (len > 0 && tokens[0][len - 1] == ':')
            {
                // Remove ':' do label
                tokens[0][len - 1] = '\0';
                label = tokens[0];
                if (word_count > 1)
                {
                    opr = tokens[1];
                    if (word_count > 2)
                        arg1 = tokens[2];
                    if (word_count > 3)
                        arg2 = tokens[3];
                }
            }
            else // Nao tem label, primeiro token eh operador
            {
                opr = tokens[0];
                if (word_count > 1)
                    arg1 = tokens[1];
                if (word_count > 2)
                    arg2 = tokens[2];
            }
            // Opcional: imprimir os tokens separados para conferência
            fputs(linha, arquivoSaidaO2);
            fprintf(arquivoSaidaO2, "Label: |%s|\n", label ? label : "(null)");
            fprintf(arquivoSaidaO2, "OPR: |%s|\n", opr ? opr : "(null)");
            fprintf(arquivoSaidaO2, "Arg1: |%s|\n", arg1 ? arg1 : "(null)");
            fprintf(arquivoSaidaO2, "Arg2: |%s|\n", arg2 ? arg2 : "(null)");

            // Escreve o conteúdo da linha nos arquivos de saída

            fputs(linha, arquivoSaidaO1);
            // fputs(linha, arquivoSaidaO2);

            // for (int i = 0; i < word_count; i++)
            // {
            //     free(tokens[i]);
            // }

            if (label)
            {
                int labelInTS = findSimboloTabela(tabelaSimbolos, qtd_simbolos, label);
                if (labelInTS != -1)
                {
                    // ERRO, REDEFINICAO DE ROTULO
                } else
                {
                    insertSimboloTabela(tabelaSimbolos, &qtd_simbolos, MAX_QTD_SIMBOLOS, label, (codigo.tamanho + 1), 1);
                }
            }
            if (opr)
            {
                if (opr == "SPACE")
                {
                    if (!label)
                    {
                        // ERRO, SEM LABEL
                    }
                    if (arg1)
                    {
                        int arg = atoi(arg1);
                        for (size_t i = 0; i < arg; i++)
                        {
                            insertInCodigoObj(&codigo, 0);
                        }
                        
                    } else
                    {
                        insertInCodigoObj(&codigo, 0);
                    }
                } else if (opr == "CONST")
                {
                    if (!label)
                    {
                        // ERRO, SEM LABEL
                    }
                    if (arg1)
                    {
                        insertInCodigoObj(&codigo, atoi(arg1));
                    } else
                    {
                        // ERRO, ARG FALTANDO
                    }
                } else
                {
                    int opcode = mnemonicTable[findMnemonic(opr, &mnemonicTable, qtd_mnemonicos)].opcode;
                    insertInCodigoObj(&codigo, opcode);
                    if (arg1)
                    {
                        int arg1InTS = findSimboloTabela(tabelaSimbolos, qtd_simbolos, arg1);
                        if (arg1InTS == -1)
                        {
                            int novo_simb = insertSimboloTabela(tabelaSimbolos, &qtd_simbolos, MAX_QTD_SIMBOLOS, arg1, 0, 0);
                            insertPendenciaAtSimbolo(novo_simb, (codigo.tamanho + 1));
                            insertInCodigoObj(&codigo, 0); // Com aritmética de ponteiros, o 0 aqui vira o número sendo somado
                        } else if (tabelaSimbolos[arg1InTS].def)
                        {
                            insertPendenciaAtSimbolo(&tabelaSimbolos[arg1InTS], (codigo.tamanho + 1));
                            insertInCodigoObj(&codigo, 0); // Com aritmética de ponteiros, o 0 aqui vira o número sendo somado
                        } else
                        {
                            insertInCodigoObj(&codigo, tabelaSimbolos[arg1InTS].valor);
                        }
                    }
                    if (arg2)
                    {
                        int arg2InTS = findSimboloTabela(tabelaSimbolos, qtd_simbolos, arg2);
                        if (arg2InTS == -1)
                        {
                            int novo_simb = insertSimboloTabela(tabelaSimbolos, &qtd_simbolos, MAX_QTD_SIMBOLOS, arg2, 0, 0);
                            insertPendenciaAtSimbolo(novo_simb, (codigo.tamanho + 1));
                            insertInCodigoObj(&codigo, 0); // Com aritmética de ponteiros, o 0 aqui vira o número sendo somado
                        } else if (tabelaSimbolos[arg2InTS].def)
                        {
                            insertPendenciaAtSimbolo(&tabelaSimbolos[arg2InTS], (codigo.tamanho + 1));
                            insertInCodigoObj(&codigo, 0); // Com aritmética de ponteiros, o 0 aqui vira o número sendo somado
                        } else
                        {
                            insertInCodigoObj(&codigo, tabelaSimbolos[arg2InTS].valor);
                        }
                    }
                }
            }
            
            /* 
            if (label)
            {
                add label to TS, like this:
                try to find label in TS
                if failed:
                    insertSimboloTabela(tabela, n?(perguntar o que é), maxTable, label, CodigoObj.tamanho+1, 1) PERGUNTAR SOBRE IMPLEMENTACAO EXATA DA FUNCAO; PODE DAR ERRO TENTAR ACESSAR O CODIGOOBJ AQUI
                if succeeded:
                    ERRO, ROTULO JA DEFINIDO
            }
            if (opr)
            {
                if (opr == SPACE)
                {
                    if (!label)
                    {
                        ERRO, SEM LABEL
                    }
                    if (arg1)
                    {
                        transform arg1 to int
                        for (i = 0, i < arg1, i++)
                        {
                            insertInCodigoObj(0)
                        }
                    } else
                    {
                        insertInCodigoObj(0)
                    }
                } else if (opr == CONST)
                {
                    if (!label)
                    {
                        ERRO, SEM LABEL
                    }
                    if (arg1)
                    {
                        insertInCodigoObj(arg1)
                    } else
                    {
                        ERRO, ARG FALTANDO
                    }
                } else
                {
                    turn opr String to Opcode using Mnemonics table
                    insertInCodigoObj(opcode)
                    if (arg1)
                    {
                        read arg1's label
                        try to find arg1's label from TS
                        if doesn't exist:
                            add to TS, start Lista de pendencias with current address, insertInCodigoObj(either 0 or the number added to the label), like this:
                            insertSimboloTabela(tabela, n???, maxTable, arg1, 0 [valor temp], 0)
                            insertPendenciaAtSimbolo(numero do simbolo que acabamos de criar, endereco atual)
                            insertInCodigoObj(0)
                        else if exists but is not defined:
                            add current address to Lista de pendencias, insertInCodigoObj(either 0 or the number added to the label), like this:
                            insertPendenciaAtSimbolo(numero do simbolo, endereco atual)
                            insertInCodigoObj(0)
                        else if exists and is defined:
                            get label address and insertInCodigoObj(address)

                    }
                }
            }
            */
        }
    }
    for (size_t i = 0; i < qtd_simbolos; i++)
    {
        TabelaSimbolo linhaTab = tabelaSimbolos[i];
        if (linhaTab.def == 0)
        {
            for (size_t j = 0; j < linhaTab.num_pendencias; j++)
            {
                addToAddressInCodigoObj(&codigo, j, linhaTab.valor);
            }
        }
    }
    
    /*
    Logica de resolucao da lista de pendencias
    for i in range(qtd_simbolos):
        linha = tabelaSimbolos[i]
        if linha.def == 0:
            for j in linha.pendencias:
                addToAddressInCodigoObj(codigo, j, linha.valor)
    */
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

    arquivoEntrada = fopen(nomeArquivoEntrada, "r");
    if (arquivoEntrada == NULL)
    {
        perror("Erro ao abrir o arquivo de entrada");
        return EXIT_FAILURE;
    }
    arquivoSaidaO1 = fopen(nomeArquivoSaidaO1, "w");
    if (arquivoSaidaO1 == NULL)
    {
        perror("Erro ao abrir o arquivo de saida");
        fclose(arquivoEntrada);
        return EXIT_FAILURE;
    }
    arquivoSaidaO2 = fopen(nomeArquivoSaidaO2, "w");
    if (arquivoSaidaO2 == NULL)
    {
        perror("Erro ao abrir o arquivo de saida");
        fclose(arquivoEntrada);
        fclose(arquivoSaidaO1);
        return EXIT_FAILURE;
    }

    printf("--- Conteudo de %s (impresso no console) ---\n", nomeArquivoEntrada);

    // Chamada para a função de processamento
    compileFile(arquivoEntrada, arquivoSaidaO1, arquivoSaidaO2);

    printf("------------------------------------------------\n");
    printf("Arquivo lido e copiado de %s para %s com sucesso!\n",
           nomeArquivoEntrada, nomeArquivoSaidaO1);
    printf("Arquivo lido e copiado de %s para %s com sucesso!\n",
           nomeArquivoEntrada, nomeArquivoSaidaO2);

    fclose(arquivoEntrada);
    fclose(arquivoSaidaO1);
    fclose(arquivoSaidaO2);

    return 0;
}
