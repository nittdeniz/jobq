#include <errno.h>
#include <math.h>
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

void long2char(long number, char* str, int len){
    if( number < 0 ){
        return;
    }
    for( int i = 0; i < len; i++ ){
        str[len-1-i] = (char)(number%10);
        str[len-1-i] += '0';
        number /= 10;
    }
}

long char2long(const char* str, int len){
    long result = 0;
    for( int i = 0; i < len; i++ ){
        result += (str[len-1-i] - '0') * (long)pow(10L, i);
    }
    return result;
}


void quit(const char* msg){
    printf("%s", msg);
    exit(EXIT_FAILURE);
}

void quit_with_error(const char* msg){
    perror(msg);
    exit(EXIT_FAILURE);
}