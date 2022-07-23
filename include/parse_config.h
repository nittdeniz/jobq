//
// Created by nittdeniz on 7/23/22.
//

#ifndef JOBQ_PARSE_CONFIG_H
#define JOBQ_PARSE_CONFIG_H

struct Config{
    long port;
};

void parse_config(const char* file_name, struct Config* config);

#endif //JOBQ_PARSE_CONFIG_H
