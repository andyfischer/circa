// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

void tar_read_file(caValue* tarBlob, const char* filename, caValue* fileOut);
bool tar_file_exists(caValue* tarBlob, const char* filename);
void tar_debug_dump_listing(caValue* tarBlob);

} // namespace circa
