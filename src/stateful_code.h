// Copyright 2009 Paul Hodge

#include "common_headers.h"

namespace circa {

// Returns whether this term is 'stateful'. The value of these terms should be persisted
// across a reload.
bool is_stateful(Term* term);

void set_stateful(Term* term, bool value);

void load_state_into_branch(Branch& state, Branch& branch);
void persist_state_from_branch(Branch& branch, Branch& state);
void get_type_from_branches_stateful_terms(Branch& branch, Branch& type);

// If a function has 'hidden state', it means that every call to this function should
// create a hidden stateful value term, and it should be passed as a hidden 0th argument.
// The function call will probably modify this term during its call.
bool has_hidden_state(Function& func);

Term* get_hidden_state_for_call(Term* term);

} // namespace circa
