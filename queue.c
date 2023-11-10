#include "queue.h"

void qinit(queue *item)
{
    item->first = NULL;
    item->last = NULL;
}

void qput(queue *item, particle data)
{
    if(!item->first) {
        item->first = malloc(sizeof(struct node));
        item->last = item->first;
    } else {
        item->last->next = malloc(sizeof(struct node));
        item->last = item->last->next;
    }
    item->last->data = data;
    item->last->next = NULL;
}

void qget(queue *item, particle *data)
{
    if(data)
        *data = item->first->data;

    struct node *tmp = item->first;
    item->first = item->first->next;
    if(!item->first)
        item->last = NULL;
    free(tmp);
}

int qempty(queue *item)
{
    return !item->first;
}
