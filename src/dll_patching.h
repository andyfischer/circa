// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

#include "common_headers.h"

namespace circa {

void unload_dll(const char* dll_filename);
void patch_with_dll(const char* dll_filename, Branch& branch);

}
