// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

List* modules_get_search_paths();

void modules_add_search_path(const char* str);

Branch* load_module_from_file(const char* module_name, const char* filename);

// returns either Success or FileNotFound
caName load_module(const char* module_name, Term* loadCall);

Branch* find_module_from_filename(const char* filename);

} // namespace circa
