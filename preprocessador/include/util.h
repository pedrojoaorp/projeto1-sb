#ifndef UTIL_H
#define UTIL_H

void trim_line(char *str);
void clean_parameter(char *param);
void extract_indentation(char *line, char *indentation);
int is_macro_related_line(char *line, int in_macro);
void remove_ampersands_from_line(char *line);
int extract_label(char *line, char *label_out, char *remaining_line);

#endif
