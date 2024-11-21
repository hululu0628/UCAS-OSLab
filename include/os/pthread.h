#ifndef INCLUDE_PTHREAD_H
#define INCLUDE_PTHREAD_H

#include <type.h>

#define THREAD_STACK 0x10000

void pthread_create(pthread_t *thread,
                   void (*start_routine)(void*),
                   void *arg);

int pthread_join(pthread_t thread);

#endif