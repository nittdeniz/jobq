#include "job.h"

#include <malloc.h>

struct Elem* push_back(struct Job_Queue* queue, struct Job j){
    struct Elem *elem = malloc(sizeof(struct Elem));
    elem->job = j;
    elem->next = NULL;
    elem->prev = NULL;
    if( queue->first == NULL ){
        queue->first = elem;
        queue->last = elem;
    }else{
        elem->prev = queue->last;
        queue->last->next = elem;
        queue->last = elem;
    }
    return elem;
}

struct Elem* delete(struct Job_Queue* queue, struct Elem* elem){
    if( elem == NULL ){
        return elem;
    }
    if( queue->first == elem ){
        queue->first = elem->next;
    }
    if( queue->last == elem ){
        queue->last = elem->prev;
    }
    if( elem->prev != NULL ){
        elem->prev->next = elem->next;
    }
    if( elem->next != NULL ){
        elem->next->prev = elem->prev;
    }
    struct Elem* next = elem->next;
    free(elem);
    elem = NULL;
    return next;
}