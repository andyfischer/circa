// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

List* modules_get_search_paths();

void modules_add_search_path(const char* str);

// returns either Success or FileNotFound
Symbol load_module(Symbol module_name, Term* loadCall);

} // namespace circa
