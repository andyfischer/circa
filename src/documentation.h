// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

std::string escape_newlines(std::string s);
void hide_from_docs(Term* term);
void append_package_docs(std::stringstream& out, Branch& branch, std::string const& package_name);

} // namespace circa
