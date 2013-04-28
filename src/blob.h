// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

void blob_setup_type(Type* type);

int blob_size(caValue* blob);

void blob_resize(caValue* blob, int size);
void blob_append_char(caValue* blob, char c);
void blob_append_int(caValue* blob, int i);

bool is_blob(caValue* val);
char* as_blob(caValue* val);
void set_blob(caValue* val, int length);

} // namespace circa
