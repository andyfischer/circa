// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

void create_function_vectorized_vs(Block* out, Term* func, Type* lhsType, Type* rhsType);
void create_function_vectorized_vv(Block* out, Term* func, Type* lhsType, Type* rhsType);

Term* create_overloaded_function(Block* block, const char* header);
void append_to_overloaded_function(Block* func, Term* function);
void append_to_overloaded_function(Term* overloadedFunc, Term* specializedFunc);
void finish_building_overloaded_function(Term* overloadedFunc);
bool is_overloaded_function(Block* block);
void list_overload_contents(Block* block, caValue* out);
Term* statically_specialize_overload_for_call(Term* call);

} // namespace circa
