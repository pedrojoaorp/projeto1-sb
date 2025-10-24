#include "./defines/defines.h"

// structs.h
#ifndef STRUCTS_H
#define STRUCTS_H

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

#endif