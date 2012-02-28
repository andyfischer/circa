// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

namespace circa {

bool is_function_stateful(Term* func);
void on_stateful_function_call_created(Term* call);
void pack_any_open_state_vars(Branch* branch);

bool is_declared_state(Term* term);
bool has_implicit_state(Term* term);
void get_type_from_branches_stateful_terms(Branch* branch, Branch* type);
void describe_state_shape(Branch* branch, caValue* output);

void strip_orphaned_state(caValue* description, caValue* state,
        caValue* trash);
void strip_orphaned_state(Branch* branch, caValue* state, caValue* trash);
void strip_orphaned_state(Branch* branch, caValue* state);

} // namespace circa
