// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

void metaprogramming_install_functions(Branch* kernel);

Term* translate_term_across_branches(Term* term, Branch* oldBranch, Branch* newBranch);
void update_all_code_references(caValue* value, Branch* oldBranch, Branch* newBranch);
void get_relative_name(Term* term, Branch* relativeTo, caValue* nameOutput);
Term* find_from_relative_name(caValue* name, Branch* relativeTo);

void set_term_ref(caValue* val, Term* term);
Term* as_term_ref(caValue* val);

} // namespace circa
