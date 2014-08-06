// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

// FileSource reading.
caValue* file_source_read_file(caValue* file_source, caValue* name);
bool file_source_does_file_exist(caValue* file_source, caValue* name);
int file_source_get_file_version(caValue* file_source, caValue* name);

// FileSource creation.
void file_source_create_using_filesystem(caValue* file_source, const char* rootDir);
void file_source_create_from_tarball(caValue* file_source, caValue* tarball);
void file_source_create_in_memory(Value* file_source);

} // namespace circa
