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
