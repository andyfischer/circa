// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#ifndef CIRCA_THREAD_H_INCLUDED
#define CIRCA_THREAD_H_INCLUDED

#include "circa.h"

// Public API

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*caThreadMainFunc)(void* data);

typedef struct caMutex;

void circa_thread_create(caThreadMainFunc func, void* data);
caMutex* circa_create_mutex();
void circa_destroy_mutex(caMutex*);
void circa_thread_mutex_lock(caMutex* mutex);
void circa_thread_mutex_unlock(caMutex* mutex);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
