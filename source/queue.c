#include "job.h"
#include "manager.h"
#include "queue.h"
#include "util.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>


void *queue(void *pointers){
    void** p = (void**) pointers;
    struct Manager* m = p[2];

    while( true ){
        fprintf(stderr, "clear\n");
        clear_finished_and_overdue_jobs(m);
        fprintf(stderr, "start\n");
        start_jobs(m);
        sleep(1);
    }
    return NULL;
}