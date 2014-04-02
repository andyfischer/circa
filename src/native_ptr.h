// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

namespace circa {

bool is_native_ptr(caValue* val);
void* as_native_ptr(caValue* val);
void set_native_ptr(caValue* val, void* ptr, caNativePtrRelease release);
void native_ptr_setup_type(Type* type);

} // namespace circa
