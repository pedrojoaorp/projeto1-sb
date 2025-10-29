#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/structs.h"
#include "../include/util.h"
#include "../include/macro.h"

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
