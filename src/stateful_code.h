// Copyright 2009 Paul Hodge

#include "common_headers.h"

namespace circa {

// Returns whether this term is 'stateful'. The value of these terms should be persisted
// across a reload.
bool is_stateful(Term* term);

void set_stateful(Term* term, bool value);

bool is_function_stateful(Term* func);

void load_state_into_branch(Branch& state, Branch& branch);
void persist_state_from_branch(Branch& branch, Branch& state);
void get_type_from_branches_stateful_terms(Branch& branch, Branch& type);

Term* get_hidden_state_for_call(Term* term);
bool terms_match_for_migration(Term* left, Term* right);

void reset_stateful_values(Branch& branch);

void migrate_stateful_values(Branch& source, Branch& dest);

} // namespace circa
