// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

void selector_prepend(caValue* selector, caValue* element);

Term* find_accessor_head_term(Term* accessor);

caValue* get_with_selector(caValue* root, caValue* selector, caValue* error);
void set_with_selector(caValue* root, caValue* selector, caValue* newValue, caValue* error);

void selector_setup_funcs(Branch* kernel);

} // namespace circa
