// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

void metaprogramming_install_functions(Branch* kernel);

void update_all_code_references_in_value(caValue* value, Branch* oldBranch, Branch* newBranch);

void set_term_ref(caValue* val, Term* term);
Term* as_term_ref(caValue* val);
bool is_term_ref(caValue* val);

} // namespace circa
