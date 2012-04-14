// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

// Runs the file checker and writes any problems to 'errors'. If 'errors' is empty
// then the check succeeded. 'filename' must be a file, not a directory.
void run_file_checker(const char* filename, List* errors);

// Runs the file checker, prints output to stdout, and returns 0 if successful.
//
// 'filename' can be a file or a directory. If it's a directory, we'll recursively
// check every *.ca file inside the directory.
int run_file_checker(const char* filename);

} // namespace circa
