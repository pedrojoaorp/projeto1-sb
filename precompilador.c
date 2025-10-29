#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// structs.h
#define STRUCTS_H
#define MAX_MACRO_NAME 64
#define MAX_MACRO_ARGS 2
#define MAX_MACROS 2
#define MAX_MACRO_LINES 100
#define MAX_LINE_SIZE 512
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

typedef struct
{
    char name[MAX_MACRO_NAME];
    char args[MAX_MACRO_ARGS][MAX_MACRO_NAME];
    int arg_count;
    char lines[MAX_MACRO_LINES][MAX_LINE_SIZE];
    int line_count;
} Macro;

typedef struct
{
    Macro macros[MAX_MACROS];
    int macro_count;
} MacroTable;

// Remove espaços e tabs do início e fim
void trim_line(char *str)
{
    int start = 0;
    int len = (int)strlen(str);
    int end = len - 1;

    /* usar (unsigned char) ao passar para isspace para evitar comportamento indefinido */
    while (start < len && (isspace((unsigned char)str[start]) || str[start] == '\t'))
        start++;

    while (end >= 0 && (isspace((unsigned char)str[end]) || str[end] == '\t' || str[end] == ','))
        end--;

    str[end + 1] = '\0';
    if (start > 0)
        memmove(str, str + start, strlen(str + start) + 1);
}

// Remove & do início do parâmetro se presente
void clean_parameter(char *param)
{
    trim_line(param);
    if (strlen(param) > 0 && param[0] == '&')
        memmove(param, param + 1, strlen(param));
}

// Extrai indentação do começo da linha
void extract_indentation(char *line, char *indentation)
{
    int i = 0;
    indentation[0] = '\0';
    // while (line[i] && (line[i] == ' ' || line[i] == '\t'))
    // {
    //     indentation[i] = line[i];
    //     i++;
    // }
    // indentation[i] = '\0';
}

// Verifica se linha relacionada a bloco de macro (inclui comentários)
int is_macro_related_line(char *line, int in_macro)
{
    char temp[MAX_LINE_SIZE];
    strcpy(temp, line);
    trim_line(temp);

    if (in_macro)
    {
        return 1;
    }

    if (strlen(temp) > 0 && temp[0] == ';')
    {
        char upper[MAX_LINE_SIZE];

        for (int i = 0; temp[i]; i++)
        {
            upper[i] = toupper(temp[i]);
        }

        upper[strlen(temp)] = '\0';
        if (strstr(upper, "MACRO") != NULL)
        {
            return 1;
        }
    }
    return 0;
}

// Remove & da linha expandida
void remove_ampersands_from_line(char *line)
{
    char result[MAX_LINE_SIZE];
    char *src = line;
    char *dst = result;

    while (*src)
    {
        if (*src == '&')
        {
            src++;
        }
        else
        {
            *(dst++) = *(src++);
        }
    }

    *dst = '\0';
    strcpy(line, result);
}

int extract_label(char *line, char *label_out, char *remaining_line)
{
    char *colon_pos = strchr(line, ':');

    if (colon_pos != NULL)
    {
        // Verifica se há apenas espaços antes dos dois pontos
        int i = 0;
        while (line[i] == ' ' || line[i] == '\t')
            i++;

        int label_start = i;
        while (line[i] != ':' && line[i] != ' ' && line[i] != '\t' && line[i] != '\0')
            i++;

        if (line[i] == ':' || (line[i] == ' ' && strchr(line + i, ':') != NULL))
        {
            // Extrair label
            int len = colon_pos - line - label_start;
            strncpy(label_out, line + label_start, len);
            label_out[len] = '\0';

            // Extrair resto da linha
            strcpy(remaining_line, colon_pos + 1);
            return 1;
        }
    }

    label_out[0] = '\0';
    strcpy(remaining_line, line);
    return 0;
}

// Busca macro por nome
int find_macro(MacroTable *table, char *name)
{
    for (int i = 0; i < table->macro_count; i++)
    {
        if (strcasecmp(table->macros[i].name, name) == 0)
            return i;
    }
    return -1;
}

// Verifica se é chamada de macro na linha, extrai indentação, label e args
int is_macro_call(char *line, MacroTable *table, int *macro_index, char *label_part, char *args_part, char *indentation)
{
    char temp[MAX_LINE_SIZE];
    strcpy(temp, line);
    extract_indentation(line, indentation);
    trim_line(temp);

    if (strlen(temp) == 0 || temp[0] == ';')
    {
        return 0;
    }

    label_part[0] = '\0';
    args_part[0] = '\0';

    char *colon = strchr(temp, ':');
    char *instruction_start = temp;

    if (colon)
    {
        *colon = '\0';
        strcpy(label_part, temp);
        strcat(label_part, ": ");

        instruction_start = colon + 1;
        trim_line(instruction_start);
    }

    char instruction_copy[MAX_LINE_SIZE];
    strcpy(instruction_copy, instruction_start);
    char *first_word = strtok(instruction_copy, " \t");

    if (!first_word)
    {
        return 0;
    }

    int idx = find_macro(table, first_word);

    if (idx >= 0)
    {
        *macro_index = idx;
        char *args_start = strstr(instruction_start, first_word);
        if (args_start)
        {
            args_start += strlen(first_word);
            trim_line(args_start);
            strcpy(args_part, args_start);
        }
        return 1;
    }

    return 0;
}

// Verifica se linha é definição de macro
int is_macro_definition(char *line)
{
    char temp[MAX_LINE_SIZE];
    strcpy(temp, line);
    trim_line(temp);

    if (strlen(temp) == 0)
    {
        return 0;
    }

    char upper[MAX_LINE_SIZE];

    for (int i = 0; temp[i]; i++)
    {
        upper[i] = toupper(temp[i]);
    }

    upper[strlen(temp)] = '\0';

    char *colon = strchr(temp, ':');
    if (colon)
    {
        char *macro_word = strstr(upper + (colon - temp), "MACRO");

        return (macro_word != NULL);
    }

    return (strncmp(upper, "MACRO", 5) == 0);
}

// Verifica se linha é fim de macro
int is_macro_end(char *line)
{
    char temp[MAX_LINE_SIZE];
    strcpy(temp, line);
    trim_line(temp);

    for (int i = 0; temp[i]; i++)
    {
        temp[i] = toupper(temp[i]);
    }

    return (strcmp(temp, "ENDMACRO") == 0 || strcmp(temp, "MEND") == 0);
}

// Substitui os argumentos da macro na linha
void substitute_arguments(char *line, Macro *macro, char call_args[][MAX_MACRO_NAME], int call_arg_count)
{
    char result[MAX_LINE_SIZE];
    strcpy(result, line);

    for (int i = 0; i < macro->arg_count && i < call_arg_count; i++)
    {
        char temp[MAX_LINE_SIZE];
        strcpy(temp, result);

        char *pos = temp;
        result[0] = '\0';

        while ((pos = strstr(pos, macro->args[i])) != NULL)
        {
            int valid = 1;
            if (pos > temp && (isalnum(*(pos - 1)) || *(pos - 1) == '_'))
            {
                valid = 0;
            }
            if (isalnum(*(pos + strlen(macro->args[i]))) || *(pos + strlen(macro->args[i])) == '_')
            {
                valid = 0;
            }
            if (valid)
            {
                strncat(result, temp, pos - temp);
                strcat(result, call_args[i]);

                strcpy(temp, pos + strlen(macro->args[i]));
                pos = temp;
            }
            else
            {
                strncat(result, temp, pos - temp + 1);
                strcpy(temp, pos + 1);
                pos = temp;
            }
        }

        strcat(result, temp);
    }

    strcpy(line, result);
}

void expand_macro(MacroTable *table, int macro_idx, char *args_string, FILE *output, char *label, char *base_indentation, int depth)
{
    if (depth > 10)
    {
        printf("ERRO: Recursao muito profunda na macro '%s'\n", table->macros[macro_idx].name);
        return;
    }

    Macro *macro = &table->macros[macro_idx];
    char call_args[MAX_MACRO_ARGS][MAX_MACRO_NAME];
    int call_arg_count = 0;

    if (strlen(args_string) > 0)
    {
        char args_copy[MAX_LINE_SIZE];
        strcpy(args_copy, args_string);
        char *token = strtok(args_copy, ",");
        while (token && call_arg_count < MAX_MACRO_ARGS)
        {
            trim_line(token);
            if (strlen(token) > 0)
            {
                strcpy(call_args[call_arg_count], token);
                call_arg_count++;
            }
            token = strtok(NULL, ",");
        }
        if (call_arg_count == 0)
        {
            strcpy(args_copy, args_string);
            token = strtok(args_copy, " \t");
            while (token && call_arg_count < MAX_MACRO_ARGS)
            {
                trim_line(token);
                if (strlen(token) > 0)
                {
                    strcpy(call_args[call_arg_count], token);
                    call_arg_count++;
                }
                token = strtok(NULL, " \t");
            }
        }
    }
    for (int i = 0; i < macro->line_count; i++)
    {
        char expanded_line[MAX_LINE_SIZE];

        strcpy(expanded_line, macro->lines[i]);
        substitute_arguments(expanded_line, macro, call_args, call_arg_count);
        remove_ampersands_from_line(expanded_line);

        char inner_label[MAX_MACRO_NAME], inner_args[MAX_LINE_SIZE], inner_indentation[64];
        int inner_macro_idx;

        if (is_macro_call(expanded_line, table, &inner_macro_idx, inner_label, inner_args, inner_indentation))
        {
            char full_label[MAX_LINE_SIZE];
            if (i == 0 && strlen(label) > 0)
            {
                strcpy(full_label, label);
                strcat(full_label, inner_label);
            }
            else
            {
                strcpy(full_label, inner_label);
            }

            expand_macro(table, inner_macro_idx, inner_args, output, full_label, base_indentation, depth + 1);
        }
        else
        {
            if (i == 0 && strlen(label) > 0)
            {
                char trimmed[MAX_LINE_SIZE];
                strcpy(trimmed, expanded_line);
                trim_line(trimmed);

                if (!(strlen(trimmed) > 0 && trimmed[0] == ';'))
                {
                    fprintf(output, "%s%s%s\n", base_indentation, label, trimmed);
                }
            }
            else
            {
                char trimmed[MAX_LINE_SIZE];
                strcpy(trimmed, expanded_line);
                trim_line(trimmed);

                if (!(strlen(trimmed) > 0 && trimmed[0] == ';'))
                {
                    fprintf(output, "%s%s\n", base_indentation, trimmed);
                }
            }
        }
    }
}

// Parseia a definição da macro
void parse_macro_definition(char *line, Macro *macro)
{
    char temp[MAX_LINE_SIZE];
    strcpy(temp, line);
    memset(macro, 0, sizeof(Macro));
    char *colon = strchr(temp, ':');

    if (colon)
    {
        *colon = '\0';

        strcpy(macro->name, temp);
        trim_line(macro->name);

        char *macro_part = colon + 1;
        char *macro_word = strstr(macro_part, "MACRO");

        if (!macro_word)
        {
            macro_word = strstr(macro_part, "macro");
        }

        if (macro_word)
        {
            char *args_start = macro_word + 5;

            trim_line(args_start);

            if (strlen(args_start) > 0)
            {
                char *token = strtok(args_start, ", \t");
                while (token && macro->arg_count < MAX_MACRO_ARGS)
                {
                    clean_parameter(token);
                    if (strlen(token) > 0)
                    {
                        strcpy(macro->args[macro->arg_count], token);
                        macro->arg_count++;
                    }
                    token = strtok(NULL, ", \t");
                }
            }
        }
    }
}

// Função principal de pré-processamento
int preprocess(char *input_filename, char *output_filename)
{
    FILE *input = fopen(input_filename, "r");
    FILE *output = fopen(output_filename, "w");

    if (!input)
    {
        printf("ERRO: Nao foi possivel abrir o arquivo %s\n", input_filename);
        return -1;
    }
    if (!output)
    {
        printf("ERRO: Nao foi possivel criar o arquivo %s\n", output_filename);
        if (input)
            fclose(input);
        return -1;
    }

    MacroTable table;

    memset(&table, 0, sizeof(table));
    char line[MAX_LINE_SIZE];
    int in_macro_definition = 0;
    int current_macro_index = -1;
    int have_code_started = 0;
    char currentLabel[MAX_LINE_SIZE];
    char lastLabel[MAX_LINE_SIZE];
    char line_no_label[MAX_LINE_SIZE];
    int has_label = 0;
    int label_found = 0;

    while (fgets(line, sizeof(line), input))
    {
        char temp_line[MAX_LINE_SIZE];
        line[strcspn(line, "\r\n")] = '\0';

        if (strlen(line) > 0 && line[0] == ';')
        {
            continue; // Pular todo comentario
        }

        if (strlen(line) == 0)
        {
            continue;
        }

        if (!have_code_started && strlen(line) == 0)
        {
            continue;
        }
        else
        {
            have_code_started = 1;
        }

        // Extrai label da linha
        label_found = extract_label(line, currentLabel, line_no_label);
        int has_opr = strlen(line_no_label) > 0 ? 1 : 0;

        if (!has_label && label_found && !has_opr)
        {
            strcpy(lastLabel, currentLabel);
            has_label = 1;
        }

        // printf("\nlinha:%s  -> %d, %d line_no_label: %d\n", line, label_found, has_label, strlen(line_no_label));

        if (has_label && !has_opr)
        {
            continue;
        }

        if (has_label)
        {
            if (!label_found && has_label && has_opr)
            {
                snprintf(temp_line, sizeof(temp_line), "%s: %s", lastLabel, line_no_label);
                strcpy(line, temp_line);
                label_found = 0;
                has_label = 0;
                lastLabel[0] = '\0';
            }
        }
        else
        {
            label_found = 0;
            has_label = 0;
        }

        if (is_macro_definition(line))
        {
            if (table.macro_count < MAX_MACROS)
            {
                current_macro_index = table.macro_count;
                parse_macro_definition(line, &table.macros[current_macro_index]);
                if (strlen(table.macros[current_macro_index].name) > 0)
                {
                    table.macro_count++;
                    in_macro_definition = 1;
                }
            }
        }

        else if (is_macro_end(line))
        {
            if (in_macro_definition && current_macro_index >= 0)
            {
                in_macro_definition = 0;
                current_macro_index = -1;
            }
        }

        else if (in_macro_definition && current_macro_index >= 0)
        {
            if (table.macros[current_macro_index].line_count < MAX_MACRO_LINES)
            {
                strcpy(table.macros[current_macro_index].lines[table.macros[current_macro_index].line_count], line);

                table.macros[current_macro_index].line_count++;
            }
        }

        else if (is_macro_related_line(line, 0))
        {
            continue; // nunca grava comentário nem nada relacionado a macro fora do corpo
        }

        else
        {
            char label_part[MAX_LINE_SIZE], args_part[MAX_LINE_SIZE], original_indentation[64];
            int macro_index;

            if (is_macro_call(line, &table, &macro_index, label_part, args_part, original_indentation))
            {
                expand_macro(&table, macro_index, args_part, output, label_part, original_indentation, 0);
            }

            else
            {
                if (!(strlen(line) > 0 && line[0] == ';'))
                {
                    fprintf(output, "%s\n", line);
                }
            }
        }
    }

    fclose(input);
    fclose(output);

    printf("Pre-processamento concluido. %d macros processadas.\n", table.macro_count);
    return 0;
}

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

            for (int i = 0; i < word_count; i++)
            {
                free(tokens[i]);
            }
        }
    }
}

int main(int argc, char *argv[])
{
    FILE *arquivoEntrada;
    FILE *arquivoSaidaO1;
    FILE *arquivoSaidaO2;
    char linha[MAX_LINHA];

    char output_pre_filename[512];
    strcpy(output_pre_filename, argv[1]);
    char *dot = strrchr(output_pre_filename, '.');

    if (dot)
        strcpy(dot, ".pre");
    else
        strcat(output_pre_filename, ".pre");

    int result_preprocess = preprocess(argv[1], output_pre_filename);

    if (result_preprocess == 0)
    {
        printf("Arquivo .pre gerado: %s\n", output_pre_filename);

        const char *nomeArquivoEntrada = output_pre_filename;
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
            perror("Erro ao abrir o arquivo de saida O1");
            fclose(arquivoEntrada);
            return EXIT_FAILURE;
        }
        arquivoSaidaO2 = fopen(nomeArquivoSaidaO2, "w");
        if (arquivoSaidaO2 == NULL)
        {
            perror("Erro ao abrir o arquivo de saida O2");
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
    else
    {
        printf("Erro durante o pre-processamento.\n");
    }

    // Verificar qtd de argumentos
    if (argc != 2)
    {
        fprintf(stderr, "Erro com quantidade de argumentos recebidos");
        return EXIT_FAILURE;
    }
}
