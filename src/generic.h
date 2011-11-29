// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

namespace circa {

void create_function_vectorized_vs(Branch* out, Term* func, Type* lhsType, Type* rhsType);
void create_function_vectorized_vv(Branch* out, Term* func, Type* lhsType, Type* rhsType);

Term* create_overloaded_function(Branch* branch, const char* name, TermList const& functions);
Term* create_overloaded_function(Branch* branch, const char* name, TermList* functions);
void create_overloaded_function(Branch* out, TermList* functions);
void append_to_overloaded_function(Branch* func, Term* function);
void overload_post_input_change(Term* term);
bool is_overloaded_function(Function* func);

} // namespace circa
