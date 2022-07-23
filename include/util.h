#ifndef JOBQ_UTIL_H
#define JOBQ_UTIL_H

#include <stdbool.h>

bool parse_long(const char* str, long* value);

void quit(const char* msg);
void quit_with_error(const char* msg);
#endif //JOBQ_UTIL_H
