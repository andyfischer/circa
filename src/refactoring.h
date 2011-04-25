// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

// Used in change_function, find the correct EvaluateFunc for this term.
EvaluateFunc derive_evaluate_func(Term* term);
void update_cached_evaluate_func(Term* term);

// Change this term to have the given function
void change_function(Term* term, Term* newFunction);

void unsafe_change_type(Term* term, Term* type);
void change_type(Term* term, Term* type);
void respecialize_type(Term* term);
void specialize_type(Term* term, Term* type);

// Rename term, modify the name binding of the owning branch if necessary
void rename(Term* term, std::string const& name);

// Remove this term from its owning branch, and add it to this branch instead.
// This will create a NULL in the owning branch.
void steal_term(Term* term, Branch& newHome);

// Modify term so that it has the given function and inputs.
void rewrite(Term* term, Term* function, TermList const& _inputs);

// Make sure that branch[index] is a value with the given type. If that term exists and
// has a different function or type, then change it. If the branch doesn't have that
// index, then add NULL terms until it does.
void rewrite_as_value(Branch& branch, int index, Term* type);

// Remove this term from its owning branch.
void erase_term(Term* term);

} // namespace circa
