// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

#include "common_headers.h"

namespace circa {

// Returns whether this term is 'stateful'. The value of these terms should be persisted
// across a reload.
bool is_stateful(Term* term);

bool is_function_stateful(Term* func);

void load_state_into_branch(TaggedValue* state, Branch& branch);
void persist_state_from_branch(Branch& branch, TaggedValue* state);
void get_type_from_branches_stateful_terms(Branch& branch, Branch& type);

TaggedValue* get_hidden_state_for_call(Term* term);
Term* find_call_for_hidden_state(Term* term);
bool term_types_match_for_migration(Term* left, Term* right);
bool terms_match_for_migration(Term* left, Term* right);
void mark_stateful_value_assigned(Term* term);

void reset_state(Branch& branch);

void migrate_stateful_values(Branch& source, Branch& dest);

extern bool MIGRATE_STATEFUL_VALUES_VERBOSE;

} // namespace circa
