// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

void reflection_install_functions(Block* kernel);

void update_all_code_references_in_value(caValue* value, Block* oldBlock, Block* newBlock);

void set_term_ref(caValue* val, Term* term);
Term* as_term_ref(caValue* val);
bool is_term_ref(caValue* val);

} // namespace circa
