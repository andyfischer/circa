// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#if CIRCA_ENABLE_THREADING

#include <pthread.h>

#include "circa/thread.h"

typedef struct caMutex {
    pthread_mutex_t mutex;
} caMutex;

extern "C" void circa_spawn_thread(caThreadMainFunc func, void* data)
{
    // TODO
}

extern "C" caMutex* circa_create_mutex()
{
    caMutex* mutex = (caMutex*) malloc(sizeof(caMutex));
    pthread_mutex_init(&mutex->mutex, NULL);
    return mutex;
}
extern "C" void circa_destroy_mutex(caMutex* mutex)
{
    pthread_mutex_destroy(&mutex->mutex);
}


extern "C" void circa_thread_mutex_lock(caMutex* mutex)
{
    pthread_mutex_lock(&mutex->mutex);
}

extern "C" void circa_thread_mutex_unlock(caMutex* mutex)
{
    pthread_mutex_unlock(&mutex->mutex);
}

#else // CIRCA_ENABLE_THREADING

typedef struct caMutex {
} caMutex;

extern "C" void circa_spawn_thread(caThreadMainFunc func, void* data) { }
extern "C" caMutex* circa_create_mutex() { return NULL; }
extern "C" void circa_destroy_mutex(caMutex* mutex) { }
extern "C" void circa_thread_mutex_lock(caMutex* mutex) { }
extern "C" void circa_thread_mutex_unlock(caMutex* mutex) { }

#endif // CIRCA_ENABLE_THREADING
