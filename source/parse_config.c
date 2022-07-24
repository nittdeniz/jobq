#include "config.h"
#include "parse_config.h"
#include "util.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define FILE_BUFFER_SIZE 256
#define PARAMETER_SIZE 16
#define VALUE_SIZE 39

enum STATE{
    PARSE_PARAMETER,
    PARSE_VALUE,
    SAVE_IN_CONFIG
};

void parse_config(const char* file_name, struct Config *config){
    // default values
    config->maxcores = 8;
    memset(&(config->server_ip), 0, 40);
    memcpy(&(config->server_ip[0]), "127.0.0.1", 9);
    config->longjob = 86400;
    config->longmaxcores = 4;
    config->port = 8000;
    FILE* config_file = fopen(CONFIG_FILE, "r");
    if( config_file == NULL ){
        quit_with_error(CONFIG_FILE);
    }

    char buffer[FILE_BUFFER_SIZE];
    memset(&buffer, 0, FILE_BUFFER_SIZE);
    size_t read_bytes = fread(&buffer, sizeof(char), FILE_BUFFER_SIZE, config_file);
    if( ferror(config_file) ){
        quit_with_error(CONFIG_FILE);
    }

    char parameter[PARAMETER_SIZE];
    char value[VALUE_SIZE];
    memset(&parameter, 0, PARAMETER_SIZE);
    memset(&value, 0, VALUE_SIZE);

    size_t j = 0;
    enum STATE state = PARSE_PARAMETER;
    for( size_t i = 0; i < read_bytes; ++i ){
        if( buffer[i] == '=' ){
            j = 0;
            state = PARSE_VALUE;
            continue;
        }
        else if( buffer[i] == '\n' || i == read_bytes - 1 ){
            j = 0;
            state = SAVE_IN_CONFIG;
        }
        switch( state ){
            case PARSE_PARAMETER: {
                if( j >= PARAMETER_SIZE ){
                    quit("Error: Parameter name too long. Forgot `=`?\n");
                }
                parameter[j] = buffer[i];
                j++;
                break;
            }
            case PARSE_VALUE: {
                if( j >= VALUE_SIZE ){
                    quit("Error: Value too long. Forgot `\\n`?\n");
                }
                value[j] = buffer[i];
                j++;
                break;
            }
            case SAVE_IN_CONFIG:{
                {
                    if( strcmp(&parameter[0], "port") == 0 ){
                        if( !parse_long(value, &(config->port)) ){
                            quit("Could not parse port from config.");
                        }
                    }
                    else if( strcmp(&parameter[0], "host") == 0 ){
                        memcpy(config->server_ip, &value[0], strlen(&value[0]));
                    }
                    else if( strcmp(&parameter[0], "maxcores") == 0 ){
                        if( !parse_long(value, &(config->maxcores)) ){
                            quit("Could not parse maxcores from config.");
                        }
                    }
                    else if( strcmp(&parameter[0], "longjob") == 0 ){
                        if( !parse_long(value, &(config->longjob)) ){
                            quit("Could not parse longjob from config.");
                        }
                    }
                    else if( strcmp(&parameter[0], "longmaxcores") == 0 ){
                        if( !parse_long(value, &(config->longmaxcores)) ){
                            quit("Could not parse longmaxcores from config.");
                        }
                    }
                    else if( strcmp(&parameter[0], "maxtime") == 0 ){
                        if( !parse_long(value, &(config->maxtime)) ){
                            quit("Could not parse maxtime from config.");
                        }
                    }
                    else{
                        printf("Unknown parameter: %s\n", parameter);
                        exit(EXIT_FAILURE);
                    }
                    j = 0;
                    memset(&parameter, 0, PARAMETER_SIZE);
                    memset(&value, 0, VALUE_SIZE);
                    state = PARSE_PARAMETER;
                }
                break;
            }
            default:{
            }
        }
    }
}