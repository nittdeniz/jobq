#include "job.h"
#include "manager.h"
#include "queue.h"

#include <unistd.h>
#include <stdbool.h>
#include <string.h>


void *queue(void *pointers){
    void** p = (void**) pointers;
    struct Manager* m = p[2];

    while( true ){
        clear_finished_and_overdue_jobs(m);
        start_jobs(m);
        sleep(1);
    }
    return NULL;
}