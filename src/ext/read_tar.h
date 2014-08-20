// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

void tar_read_file(Value* tarBlob, const char* filename, Value* fileOut);
bool tar_file_exists(Value* tarBlob, const char* filename);
void tar_debug_dump_listing(Value* tarBlob);

} // namespace circa
