#ifndef JOBQ_MANAGER_H
#define JOBQ_MANAGER_H

#include <pthread.h>
#include <stdbool.h>
#include <time.h>

struct Job_Queue;
struct Elem;

struct Manager{
    struct Job_Queue* running_queue;
    struct Job_Queue* waiting_queue;
    struct Elem* priority_elem;
    time_t latest_end_time;
    long n_cores;
    unsigned long long int available_cores;
    pthread_mutex_t *running_lock;
    pthread_mutex_t *waiting_lock;
};

struct Elem* start_job(struct Manager* m, struct Elem* elem);
void clear_finished_and_overdue_jobs(struct Manager* m);
time_t get_latest_end_time(struct Manager* m);
long get_free_cores(struct Manager* m);
void start_jobs(struct Manager* m);
#endif //JOBQ_MANAGER_H
