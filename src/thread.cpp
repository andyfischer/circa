// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include <pthread.h>

#include "circa/thread.h"

extern "C" {

typedef struct caMutex {
    pthread_mutex_t mutex;
} caMutex;

void circa_spawn_thread(caThreadMainFunc func, void* data)
{
    // TODO
}

caMutex* circa_create_mutex()
{
    caMutex* mutex = (caMutex*) malloc(sizeof(caMutex));
    pthread_mutex_init(&mutex->mutex, NULL);
    return mutex;
}
void circa_destroy_mutex(caMutex* mutex)
{
    pthread_mutex_destroy(&mutex->mutex);
}


void circa_thread_mutex_lock(caMutex* mutex)
{
    pthread_mutex_lock(&mutex->mutex);
}

void circa_thread_mutex_unlock(caMutex* mutex)
{
    pthread_mutex_unlock(&mutex->mutex);
}

} // extern "C"
