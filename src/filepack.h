// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

// Filepack reading.
caValue* filepack_read_file(caValue* filepack, caValue* name);
bool filepack_does_file_exist(caValue* filepack, caValue* name);
int filepack_get_file_version(caValue* filepack, caValue* name);

// Filepack creation.
void filepack_create_using_filesystem(caValue* filepack, const char* rootDir);
void filepack_create_from_tarball(caValue* filepack, caValue* tarball);

} // namespace circa
