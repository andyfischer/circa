// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

namespace circa {

char* blob_data_flat(Value* blob);
u32 blob_size(Value* blob);

void set_blob(Value* value, u32 initialSize);
void set_blob_slice(Value* value, Value* backingValue, const char* data, u32 numBytes);
void set_blob_flat(Value* value, const char* data, u32 size);
void set_blob_from_str(Value* value, const char* str);

void blob_release(Value* value);
void blob_resize(Value* blob, u32 size);
char* blob_grow(Value* blob, u32 size);

void blob_slice(Value* blob, int start, int end, Value* sliceOut);
void blob_to_str(Value* blob);

void blob_setup_type(Type* type);
void blob_install_functions(NativePatch* patch);

} // namespace circa
