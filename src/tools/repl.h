// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

namespace circa {

void repl_evaluate_line(Branch* branch, std::string const& input, std::ostream& output);
int run_repl();

} // namespace circa
