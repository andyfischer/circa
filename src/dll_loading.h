// Copyright (c) Paul Hodge. See LICENSE file for license terms.

namespace circa {

void unload_dll(const char* filename);
void patch_with_dll(const char* dll_filename, Branch* branch, TaggedValue* errorOut);

} // namespace circa
