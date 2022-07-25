#ifndef JOBQ_UTIL_H
#define JOBQ_UTIL_H

#include <stdbool.h>

bool parse_long(const char* str, long* value);
void long2char(long number, char* str, int len);
long char2long(const char* str, int len);

void quit(const char* msg);
void quit_with_error(const char* msg);

char** split(char* str, char tok, int* len);
#endif //JOBQ_UTIL_H
