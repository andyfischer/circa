// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

#include "common_headers.h"

namespace circa {

// Load the dll with the given filename. Then, searches 'branch' for any functions that
// have the same name as an exported function inside the DLL. If any matches are found,
// the Circa function will have its evaluate handler overwritten with the DLL version.
//
// dll_filename should be a filename with no suffix (ie without '.so' or '.dll')
//
// If the DLL fails to load then a string message will be written to 'errorOut..
void patch_with_dll(const char* dll_filename, Branch& branch, TaggedValue* errorOut);

}
