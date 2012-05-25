// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#ifndef CIRCA_FILE_H_INCLUDED
#define CIRCA_FILE_H_INCLUDED

#include "circa.h"

#ifdef __cplusplus
extern "C" {
#endif

const char* circa_read_file(const char* filename);

bool circa_file_exists(const char* filename);

int circa_file_get_version(const char* filename);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
