#include "job.h"

#include <malloc.h>

struct Elem* push_back(struct Job_Queue* queue, struct Job j){
    fprintf(stderr, "hidden malloc\n");
    struct Elem *elem = malloc(sizeof(struct Elem));
    fprintf(stderr, "hidden malloc end\n");
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

struct Elem* erase(struct Job_Queue* queue, struct Elem** elem){
    if( queue == NULL || elem == NULL || *elem == NULL ){
        return NULL;
    }
    if( (*elem)->next != NULL ){
        (*elem)->next->prev = (*elem)->prev;
    }
    if( (*elem)->prev != NULL ){
        (*elem)->prev->next = (*elem)->next;
    }
    if( queue->first == *elem ){
        queue->first = (*elem)->next;
    }
    if( queue->last == *elem ){
        queue->last = (*elem)->prev;
    }
    struct Elem* next = (*elem)->next;
    fprintf(stderr,"free elem: %p\n", *elem);
    free(*elem);
    fprintf(stderr,"set elem to NULL: %p\n", *elem);
    *elem = NULL;
    fprintf(stderr,"set elem to NULL: %p\n", *elem);
    elem = NULL;
    return next;
}