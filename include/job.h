#ifndef JOBQ_JOB_H
#define JOBQ_JOB_H

#include "config.h"
#include <time.h>
#include <unistd.h>

struct Job{
    long id;
    pid_t pid;
    long cores;
    long time_limit;
    time_t start_time;
    time_t end_time;
    uid_t user_id;
    gid_t group_id;
    char user_name[USERNAME_BUFFER];
    char working_directory[DIRECTORY_BUFFER];
    char cmd[MAX_CMD_LENGTH];
    unsigned long long int core_mask;
};

struct Job_Queue{
    struct Elem{
        struct Job job;
        struct Elem* next;
        struct Elem* prev;
    } *first, *last;
};

struct Elem* push_back(struct Job_Queue* queue, struct Job j);
struct Elem* delete(struct Job_Queue* queue, struct Elem* elem);


#endif //JOBQ_JOB_H
