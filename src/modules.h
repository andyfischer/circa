// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

List* modules_get_search_paths();

void modules_add_search_path(const char* str);

Term* load_module_from_file(Symbol module_name, const char* filename);

// returns either Success or FileNotFound
Symbol load_module(Symbol module_name, Term* loadCall);

} // namespace circa
