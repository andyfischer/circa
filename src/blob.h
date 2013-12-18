// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

int blob_size(caValue* blob);

void blob_resize(caValue* blob, int size);
void blob_append_char(caValue* blob, char c);
void blob_append_u16(caValue* blob, u16 val);
void blob_append_u32(caValue* blob, u32 val);
void blob_append_space(caValue* blob, size_t size);

char blob_read_char(const char* data, int* pos);
u16 blob_read_u16(const char* data, int* pos);
u32 blob_read_u32(const char* data, int* pos);
void* blob_read_pointer(const char* data, int* pos);

void blob_write_u32(char* data, int* pos, u32 value);
void blob_write_pointer(char* data, int* pos, void* value);

void blob_to_hex_string(caValue* blob, caValue* str);

bool is_blob(caValue* val);
char* as_blob(caValue* val);
void set_blob(caValue* val, int length);
void set_blob_from_string(caValue* value, const char* str);
void blob_setup_type(Type* type);

} // namespace circa
