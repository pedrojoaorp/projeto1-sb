#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/structs.h"
#include "../include/util.h"
#include "../include/macro.h"
#include "../include/preprocessador.h"

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