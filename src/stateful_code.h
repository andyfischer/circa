// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

bool is_get_state(Term* term);
bool has_implicit_state(Term* term);
bool is_function_stateful(Term* func);
bool has_any_inlined_state(Branch* branch);
void mark_branch_as_having_inlined_state(Branch* branch);
void mark_branch_as_possibly_not_having_inlined_state(Branch* branch);
void get_type_from_branches_stateful_terms(Branch* branch, Branch* type);
void describe_state_shape(Branch* branch, TaggedValue* output);
void strip_orphaned_state(TaggedValue* description, TaggedValue* state,
        TaggedValue* trash);
void strip_orphaned_state(Branch* branch, TaggedValue* state, TaggedValue* trash);
void strip_orphaned_state(Branch* branch, TaggedValue* state);

} // namespace circa
