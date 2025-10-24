#ifndef MACRO_H
#define MACRO_H

#include "../include/structs.h"

int is_macro_call(char *line, MacroTable *table, int *macro_index, char *label_part, char *args_part, char *indentation);
int find_macro(MacroTable *table, char *name);
int is_macro_definition(char *line);
int is_macro_end(char *line);
void substitute_arguments(char *line, Macro *macro, char call_args[][MAX_MACRO_NAME], int call_arg_count);
void expand_macro(MacroTable *table, int macro_idx, char *args_string, FILE *output, char *label, char *base_indentation, int depth);
void parse_macro_definition(char *line, Macro *macro);

#endif
