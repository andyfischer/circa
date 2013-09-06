// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

// Returns a common type, which is guaranteed to hold all the types in this
// list. Currently, this is not very sophisticated.
Type* find_common_type(caValue* list);

// Shorthands for finding the common type of a fixed list.
Type* find_common_type(Type* type1, Type* type2);
Type* find_common_type(Type* type1, Type* type2, Type* type3);

Type* infer_type_of_get_index(Term* input);

// Create a List-based type that will have N elements, all of the same type.
Type* create_typed_unsized_list_type(Type* elementType);

} // namespace circa
