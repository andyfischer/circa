// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

// FileSource reading.
Value* file_source_read_file(Value* file_source, Value* name);
bool file_source_does_file_exist(Value* file_source, Value* name);
int file_source_get_file_version(Value* file_source, Value* name);

// FileSource creation.
void file_source_create_using_filesystem(Value* file_source, const char* rootDir);
void file_source_create_from_tarball(Value* file_source, Value* tarball);
void file_source_create_in_memory(Value* file_source);

} // namespace circa
