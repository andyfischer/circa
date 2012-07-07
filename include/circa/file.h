// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#ifndef CIRCA_FILE_H_INCLUDED
#define CIRCA_FILE_H_INCLUDED

#include "circa.h"

#ifdef __cplusplus
extern "C" {
#endif

void circa_read_file(const char* filename, caValue* contentsOut);

bool circa_file_exists(const char* filename);

int circa_file_get_version(const char* filename);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
