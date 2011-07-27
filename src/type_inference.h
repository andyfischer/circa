// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

// Returns a common type, which is guaranteed to hold all the types in this
// list. Currently, this is not very sophisticated.
Type* find_common_type(List* list);

// Guesses at the declared type for a get_index call on this term. Currently
// this is not very sophisticated, since we don't have generic types.
Type* find_type_of_get_index(Term* listTerm);

// Looks at the term's function, and generates an expression which is our
// best static guess as to the result. This might be a plain value (if the
// result is completely knowable), or it might be an expression with some
// unknowns.
Term* statically_infer_result(Branch& branch, Term* term);

// This is similar to statically_infer_result(Branch,Term), but it's used
// if you don't care about partially known values. This will either return
// a value result or :unknown.
void statically_infer_result(Term* term, TaggedValue* result);

} // namespace circa
