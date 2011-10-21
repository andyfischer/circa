// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

// Returns a common type, which is guaranteed to hold all the types in this
// list. Currently, this is not very sophisticated.
Type* find_common_type(List* list);

// Shorthands for finding the common type of a fixed list.
Type* find_common_type(Type* type1, Type* type2);
Type* find_common_type(Type* type1, Type* type2, Type* type3);

Type* infer_type_of_get_index(Term* input);

// Looks at the term's function, and generates an expression which is our
// best static guess as to the result. This might be a plain value (if the
// result is completely knowable), or it might be an expression with some
// unknowns.
Term* statically_infer_result(Branch* branch, Term* term);

// This is similar to statically_infer_result(Branch,Term), but it's used
// if you don't care about partially known values. This will either return
// a value result or :unknown.
void statically_infer_result(Term* term, TaggedValue* result);

// Create a List-based type that will have N elements, all of the same type.
Type* create_typed_unsized_list_type(Type* elementType);

} // namespace circa
