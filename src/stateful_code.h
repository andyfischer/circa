// Copyright 2009 Andrew Fischer

#include "common_headers.h"

namespace circa {

void load_state_into_branch(Term* state, Branch& branch);
void persist_state_from_branch(Branch& branch, Term* state);
void get_type_from_branches_stateful_terms(Branch& branch, Branch& type);

} // namespace circa
