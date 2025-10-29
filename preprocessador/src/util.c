#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/structs.h"
#include "../include/util.h"

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
    while (line[i] && (line[i] == ' ' || line[i] == '\t'))
    {
        indentation[i] = line[i];
        i++;
    }
    indentation[i] = '\0';
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
