#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include "util.h"

bool parse_long(const char* str, long* return_value){
    errno = 0;
    char* dummy;
    long value = strtol(str, &dummy, 0);

    if( dummy == str || *dummy != '\0' || ((value == LONG_MIN || value == LONG_MAX) && errno == ERANGE) ){
        return false;
    }
    *return_value = value;
    return true;
}

void quit(const char* msg){
    printf(msg);
    exit(EXIT_FAILURE);
}

void quit_with_error(const char* msg){
    perror(msg);
    exit(EXIT_FAILURE);
}