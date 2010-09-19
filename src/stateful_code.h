// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

#include "common_headers.h"

namespace circa {

bool is_get_state(Term* term);
bool has_implicit_state(Term* term);
bool is_function_stateful(Term* func);
bool has_any_inlined_state(Branch& branch);

void get_type_from_branches_stateful_terms(Branch& branch, Branch& type);

extern bool MIGRATE_STATEFUL_VALUES_VERBOSE;

} // namespace circa
