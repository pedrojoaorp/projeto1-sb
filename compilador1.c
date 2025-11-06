#include <stdio.h>
#include <stdlib.h> // Para EXIT_FAILURE
#include <string.h> // Para strrchr, strcmp, snprintf

#define MAX_LINHA 1024        // Para buffer de linha
#define MAX_NOME_ARQUIVO 1024 // Para nomes de arquivo
#define MAX_MNEMONIC 32
#define UPCODEFILE "upcode.txt"
#define MAX_TAMANHO_SIMBOLO 50
#define MAX_TAMANHO_MNEMONIC 50
#define MAX_PENDENCIAS 128
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
    int quantArgs;
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
    static char list[BUFFER_SIZE];
    list[0] = '\0';

    strcat(list, "LISTA DE PENDENCIAS:\n");

    for (int l = 0; l < maxLines; l++)
    {
        char temp[256];
        snprintf(temp, sizeof(temp), "Simb: %s\nValor: %d\nDef: %d\n",
                 table[l].simbolo, table[l].valor, table[l].def);
        strcat(list, temp);

        strcat(list, "Pendencias: ");

        // Itera apenas até num_pendencias, não MAX_PENDENCIAS
        for (int p = 0; p < table[l].num_pendencias; p++)
        {
            snprintf(temp, sizeof(temp), "%d", table[l].pendencias[p]);
            strcat(list, temp);
            if (p != table[l].num_pendencias - 1)
            {
                strcat(list, ", ");
            }
        }

        strcat(list, "\n\n");
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
    return ((*n) - 1); // Sucesso, retorna o ponteiro para o símbolo recém criado
}

int insertPendenciaAtSimbolo(TabelaSimbolo *simbolo, int desloc)
{
    if (simbolo->num_pendencias >= MAX_PENDENCIAS)
    {
        return 0; // Lista cheia
    }
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
        int opcode, size, quantArgs;
        // Lê formato: ADD,01,2,1
        if (sscanf(line, "%7[^,],%d,%d,%d", mnemonic, &opcode, &size, &quantArgs) == 4)
        {
            strcpy(table[count].mnemonic, mnemonic);
            table[count].opcode = opcode;
            table[count].size = size;
            table[count].quantArgs = quantArgs;
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

int writeCodObjOnFile(char *codObj, FILE *file, int lineToWrite)
{
    if (!codObj || !file || lineToWrite < 1)
        return 0;

    // Reposiciona para o início do arquivo
    rewind(file);

    char buffer[1024];
    int currentLine = 1;

    // Percorre até a linha anterior à desejada
    while (currentLine < lineToWrite && fgets(buffer, sizeof(buffer), file) != NULL)
    {
        currentLine++;
    }

    long pos = ftell(file);

    if (currentLine == lineToWrite)
    {
        // Move para a posição do início da linha que queremos sobrescrever
        fseek(file, pos, SEEK_SET);

        // Escreve a string na linha, seguida de nova linha
        fprintf(file, "%s\n", codObj);

        return 1;
    }
    else if (currentLine < lineToWrite)
    {
        // A linha desejada ainda não existe; adicionar novas linhas em branco até chegar nela
        // Posiciona no fim do arquivo
        fseek(file, 0, SEEK_END);

        while (currentLine < lineToWrite)
        {
            fprintf(file, "\n");
            currentLine++;
        }

        fprintf(file, "%s\n", codObj);
        return 1;
    }

    return 0; // Linha inválida ou erro
}

char *CodigoObjToString(CodigoObj *codObj, int maxCodObj)
{
    if (!codObj || maxCodObj <= 0)
        return NULL;

    /* estimate buffer size: allow up to 11 chars per int + 1 space, plus final NUL */
    size_t per_int = 4;
    size_t buf_size = (size_t)maxCodObj * per_int + 1;
    char *codObjString = (char *)malloc(buf_size);
    if (!codObjString)
        return NULL;

    char *p = codObjString;
    size_t remaining = buf_size;
    int written = 0;

    for (int i = 0; i < codObj->tamanho && i < maxCodObj; i++)
    {
        if (i == 0)
            written = snprintf(p, remaining, "%d", codObj->memoria[i]);
        else
            written = snprintf(p, remaining, " %d", codObj->memoria[i]);

        if (written < 0 || (size_t)written >= remaining)
        {
            /* safety: ensure NUL termination and return what we have */
            codObjString[buf_size - 1] = '\0';
            return codObjString;
        }

        p += written;
        remaining -= written;
    }

    return codObjString;
}

int resolvePendencias(TabelaSimbolo *ts, int maxTs, CodigoObj *codObj, int maxCodObj)
{
    for (int t = 0; t < maxTs; t++)
    {
        TabelaSimbolo lineSimb = ts[t];
        int valueSimb;
        if (lineSimb.def)
        {
            valueSimb = lineSimb.valor;

            for (int p = 0; p < lineSimb.num_pendencias; p++)
            {
                int pendencia = lineSimb.pendencias[p];

                /* valida índice e escreve o valor no endereço da pendência */
                if (pendencia >= 0 && pendencia < maxCodObj && pendencia < codObj->tamanho)
                {
                    codObj->memoria[pendencia] = valueSimb;
                }
            }
        }
    }

    return 1;
}

int writePendenciasOnFile(char *pendencias, FILE *file, int lineToWrite)
{
    if (!pendencias || !file || lineToWrite < 1)
        return 0;

    // Reposiciona para o início do arquivo
    rewind(file);

    char buffer[1024];
    int currentLine = 1;

    // Percorre até a linha anterior à desejada
    while (currentLine < lineToWrite && fgets(buffer, sizeof(buffer), file) != NULL)
    {
        currentLine++;
    }

    if (currentLine == lineToWrite)
    {
        // Move para a posição do início da linha que queremos sobrescrever
        long pos = ftell(file);
        fseek(file, pos, SEEK_SET);

        // Escreve a string na linha
        fprintf(file, "%s", pendencias);
        return 1;
    }
    else if (currentLine < lineToWrite)
    {
        // A linha desejada ainda não existe; adicionar novas linhas em branco até chegar nela
        fseek(file, 0, SEEK_END);

        while (currentLine < lineToWrite)
        {
            // fprintf(file, "\n");
            currentLine++;
        }

        fprintf(file, "%s", pendencias);
        return 1;
    }

    return 0;
}

int makeO1(TabelaSimbolo *ts, int maxTs, CodigoObj *codObj, int maxCodObj, FILE *arquivoSaidaO1)
{
    char *codObjString = CodigoObjToString(codObj, maxCodObj);
    char *listaPendenciasString = printListaPendencias(ts, maxTs);

    int a = writeCodObjOnFile(codObjString, arquivoSaidaO1, 1);
    int b = writePendenciasOnFile(listaPendenciasString, arquivoSaidaO1, 2);
    return 0;
}

int makeO2(TabelaSimbolo *ts, int maxTs, CodigoObj *codObj, int maxCodObj, FILE *arquivoSaidaO2)
{
    int isResolved = resolvePendencias(ts, maxTs, codObj, maxCodObj);
    char *codObjString;
    if (isResolved)
    {
        codObjString = CodigoObjToString(codObj, maxCodObj);

        if (writeCodObjOnFile(codObjString, arquivoSaidaO2, 1))
        {
            return 1;
        }
    }

    return 0;
}

int verifyLabel(char *label)
{
    // Verifica se label é NULL ou vazia
    if (label == NULL || label == '\0')
    {
        return 0; // Inválida
    }

    // Verifica primeiro caractere
    char first = label[0];
    if ((first < 'a' || first > 'z') &&
        (first < 'A' || first > 'Z') &&
        first != '_')
    {
        return 0; // Inválida - não começa com letra ou underscore
    }

    // Verifica resto dos caracteres
    for (int i = 1; label[i] != '\0'; i++)
    {
        char c = label[i];
        if ((c < 'a' || c > 'z') &&
            (c < 'A' || c > 'Z') &&
            (c < '0' || c > '9') &&
            c != '_')
        {
            return 0; // Inválida - contém caractere especial
        }
    }

    return 1; // Válida
}

int verifyQuantLabelAtLine(char *line)
{
    // Verifica se line é NULL
    if (line == NULL)
    {
        return 1; // Válido
    }

    int colonCount = 0;
    int inString = 0; // Flag para rastrear se está dentro de string

    // Percorre a linha
    for (int i = 0; line[i] != '\0'; i++)
    {
        // Ignora caracteres dentro de strings (entre aspas)
        if (line[i] == '"' && (i == 0 || line[i - 1] != '\\'))
        {
            inString = !inString;
            continue;
        }

        // Se encontrar comentário (';'), para de verificar
        if (line[i] == ';' && !inString)
        {
            break;
        }

        // Conta ':' fora de strings e comentários
        if (line[i] == ':' && !inString)
        {
            colonCount++;
        }
    }

    // Se houver 2 ou mais ':', há múltiplos rótulos
    if (colonCount >= 2)
    {
        return 0; // Inválido
    }

    return 1; // Válido
}

void compileFile(FILE *arquivoEntrada, FILE *arquivoSaidaO1, FILE *arquivoSaidaO2)
{
    char linha[1024];   // buffer para a linha lida
    char *tokens[1024]; // array para armazenar os tokens da linha
    int word_count;

    CodigoObj codigo;
    TabelaSimbolo tabelaSimbolos[MAX_QTD_SIMBOLOS];
    int qtd_simbolos = 0;
    Mnemonic mnemonicTable[MAX_MNEMONIC];
    int qtd_mnemonicos = loadMnemonics("upcode.txt", mnemonicTable, MAX_MNEMONIC);
    int current_line = 0;

    // Ler o arquivo de entrada linha por linha
    while (fgets(linha, sizeof(linha), arquivoEntrada) != NULL)
    {
        current_line++;

        if (!verifyQuantLabelAtLine(linha))
        {
            printf("-- ERRO: Multiplos rotulos na linha [%d]: [%s]\n", current_line, linha);
            continue;
        }

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

                if (!verifyLabel(label))
                {
                    printf("-- ERRO: Erro lexico na linha [%d]: [%s]\n", current_line, label);
                    continue;
                }

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

            if (label)
            {
                int labelInTS = findSimboloTabela(tabelaSimbolos, qtd_simbolos, label);
                if (labelInTS != -1)
                {
                    if (tabelaSimbolos[labelInTS].def == 0)
                    {
                        tabelaSimbolos[labelInTS].def = 1;
                        tabelaSimbolos[labelInTS].valor = (codigo.tamanho);
                    }
                    else
                    {
                        printf("-- ERRO: Rotulo [%s] sendo declarado novamente na linha [%d]\n", label, current_line); // ERRO: rotulo declarado duas vezes em lugares diferentes
                    }
                }
                else
                {
                    insertSimboloTabela(tabelaSimbolos, &qtd_simbolos, MAX_QTD_SIMBOLOS, label, (codigo.tamanho), 1);
                }
            }
            if (opr)
            {

                if (strcmp(opr, "SPACE") == 0)
                {
                    if (arg2)
                    {
                        printf("-- ERRO: Quantidade de argumentos invalida para instrucao [%s] na linha [%d]\n", opr, current_line); // ERRO: instrução com número de parâmetros errado
                    }

                    if (!label)
                    {
                        printf("-- ERRO: [%s] na linha [%d] nao tem rotulo sendo declarado\n", opr, current_line);
                    }
                    if (arg1)
                    {
                        int arg = atoi(arg1);
                        if (arg <= 0)
                        {
                            // ERRO: SPACE requer número > 0
                        }
                        for (size_t i = 0; i < arg; i++)
                        {
                            insertInCodigoObj(&codigo, 0);
                        }
                    }
                    else
                    {
                        insertInCodigoObj(&codigo, 0); // Padrão: 1 palavra
                    }
                }
                else if ((strcmp(opr, "CONST") == 0))
                {
                    if (arg2)
                    {
                        printf("-- ERRO: Quantidade de argumentos invalida para instrucao [%s] na linha [%d]\n", opr, current_line); // ERRO: instrução com número de parâmetros errado
                    }

                    if (!label)
                    {
                        printf("-- ERRO: [%s] na linha [%d] nao tem rotulo sendo declarado\n", opr, current_line);
                    }
                    if (arg1)
                    {
                        insertInCodigoObj(&codigo, atoi(arg1));
                    }
                    else
                    {
                        printf("-- ERRO: Quantidade de argumentos invalida para instrucao [%s] na linha [%d]\n", opr, current_line); // ERRO: instrução com número de parâmetros errado
                    }
                }
                else
                {
                    int opcode_index = findMnemonic(opr, mnemonicTable, qtd_mnemonicos);
                    int opcode;
                    if (opcode_index != -1)
                    {
                        opcode = mnemonicTable[opcode_index].opcode;
                    }
                    else
                    {
                        printf("-- ERRO: Instrucao na linha [%d] [%s] eh invalida\n", current_line, opr);
                        opcode = -1;
                    }

                    int arg_count = 0;
                    if (arg1)
                    {
                        arg_count++;
                    }
                    if (arg2)
                    {
                        arg_count++;
                    }
                    if (mnemonicTable[opcode_index].quantArgs != arg_count)
                    {
                        printf("-- ERRO: Quantidade de argumentos invalida para instrucao [%s] na linha [%d]\n", opr, current_line); // ERRO: instrução com número de parâmetros errado
                    }

                    insertInCodigoObj(&codigo, opcode);
                    if (arg1)
                    {
                        int arg1InTS = findSimboloTabela(tabelaSimbolos, qtd_simbolos, arg1);
                        if (arg1InTS == -1)
                        {
                            int novo_simb = insertSimboloTabela(tabelaSimbolos, &qtd_simbolos, MAX_QTD_SIMBOLOS, arg1, 0, 0);
                            insertPendenciaAtSimbolo(&tabelaSimbolos[novo_simb], (codigo.tamanho));
                            insertInCodigoObj(&codigo, 0); // Com aritmética de ponteiros, o 0 aqui vira o número sendo somado
                        }
                        else if (tabelaSimbolos[arg1InTS].def == 0)
                        {
                            insertPendenciaAtSimbolo(&tabelaSimbolos[arg1InTS], (codigo.tamanho));
                            insertInCodigoObj(&codigo, 0); // Com aritmética de ponteiros, o 0 aqui vira o número sendo somado
                        }
                        else
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
                            insertPendenciaAtSimbolo(&tabelaSimbolos[novo_simb], (codigo.tamanho));
                            insertInCodigoObj(&codigo, 0); // Com aritmética de ponteiros, o 0 aqui vira o número sendo somado
                        }
                        else if (tabelaSimbolos[arg2InTS].def == 0)
                        {
                            insertPendenciaAtSimbolo(&tabelaSimbolos[arg2InTS], (codigo.tamanho));
                            insertInCodigoObj(&codigo, 0); // Com aritmética de ponteiros, o 0 aqui vira o número sendo somado
                        }
                        else
                        {
                            insertInCodigoObj(&codigo, tabelaSimbolos[arg2InTS].valor);
                        }
                    }
                }
            }

            for (int i = 0; i < word_count; i++)
            {
                free(tokens[i]);
            }
        }
    }

    for (size_t i = 0; i < qtd_simbolos; i++)
    {
        if (tabelaSimbolos[i].def == 0)
        {
            printf("-- ERRO: Rotulo [%s] nao declarado\n", tabelaSimbolos[i].simbolo); // ERRO: rotulo não declarado
        }
    }

    char *codObjString = CodigoObjToString(&codigo, codigo.tamanho);
    char *list = printListaPendencias(tabelaSimbolos, tabelaSimbolos->num_pendencias);

    // funcao de montagem do o1
    int hasWritedO1 = makeO1(tabelaSimbolos, qtd_simbolos, &codigo, codigo.tamanho, arquivoSaidaO1);

    // funcao de montagem do o2
    int hasWritedO2 = makeO2(tabelaSimbolos, qtd_simbolos, &codigo, codigo.tamanho, arquivoSaidaO2);
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