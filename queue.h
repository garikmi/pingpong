#ifndef QUEUE_H_SENTRY
#define QUEUE_H_SENTRY

#include "types.h"

void qinit(queue *item);

void qput(queue *item, particle data);

void qget(queue *item, particle *data);

int qempty(queue *item);

#endif
