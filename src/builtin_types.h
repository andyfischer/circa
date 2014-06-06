// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

namespace circa {

int shallow_hash_func(caValue* value);

void any_setup_type(Type* type);
void bool_setup_type(Type* type);
void int_setup_type(Type* type);
void null_setup_type(Type* type);
void number_setup_type(Type* type);
void opaque_pointer_setup_type(Type* type);
void void_setup_type(Type* type);

} // namespace circa
