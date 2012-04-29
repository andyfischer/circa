// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

void metaprogramming_install_functions(Branch* kernel);

void set_term_ref(caValue* val, Term* term);
Term* as_term_ref(caValue* val);

} // namespace circa
