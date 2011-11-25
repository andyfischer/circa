// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

namespace circa {

void create_function_vectorized_vs(Branch* out, Term* func, Type* lhsType, Type* rhsType);
void create_function_vectorized_vv(Branch* out, Term* func, Type* lhsType, Type* rhsType);

Term* create_overloaded_function(Branch* branch, const char* name, TermList* functions);
void create_overloaded_function(Branch* out, TermList* functions);

} // namespace circa
