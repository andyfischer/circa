// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

void create_function_vectorized_vs(Branch* out, Term* func, Type* lhsType, Type* rhsType);
void create_function_vectorized_vv(Branch* out, Term* func, Type* lhsType, Type* rhsType);

Term* create_overloaded_function(Branch* branch, const char* header);
void append_to_overloaded_function(Branch* func, Term* function);
void append_to_overloaded_function(Term* overloadedFunc, Term* specializedFunc);
void overload_post_input_change(Term* term);
bool is_overloaded_function(Branch* branch);
void list_overload_contents(Branch* branch, caValue* out);

} // namespace circa
