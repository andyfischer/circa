// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#ifndef CIRCA_THREAD_H_INCLUDED
#define CIRCA_THREAD_H_INCLUDED

#include "circa.h"

// Public API

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*caThreadMainFunc)(void* data);

typedef struct caMutex caMutex;

void circ_spawn_thread(caThreadMainFunc func, void* data);
caMutex* circ_create_mutex();
void circ_destroy_mutex(caMutex*);
void circ_thread_mutex_lock(caMutex* mutex);
void circ_thread_mutex_unlock(caMutex* mutex);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
