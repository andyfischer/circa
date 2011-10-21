// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

// Change this term to have the given function
void change_function(Term* term, Term* newFunction);

void unsafe_change_type(Term* term, Type* type);
void change_declared_type(Term* term, Type* type);
void respecialize_type(Term* term);
void specialize_type(Term* term, Type* type);

// Rename term, modify the name binding of the owning branch if necessary
void rename(Term* term, std::string const& name);

// Modify term so that it has the given function and inputs.
void rewrite(Term* term, Term* function, TermList const& _inputs);

// Make sure that branch[index] is a value with the given type. If that term exists and
// has a different function or type, then change it. If the branch doesn't have that
// index, then add NULL terms until it does.
void rewrite_as_value(Branch* branch, int index, Type* type);

// Calls erase_term, and will also shuffle the terms inside the owning branch to
// fill in the empty index.
void remove_term(Term* term);

void remap_pointers(Term* term, TermMap const& map);
void remap_pointers(Term* term, Term* original, Term* replacement);
void remap_pointers(Branch* branch, Term* original, Term* replacement);

} // namespace circa
