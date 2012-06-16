// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

void selector_prepend(caValue* selector, caValue* element);

caValue* get_with_selector(caValue* root, caValue* selector);
void assign_with_selector(caValue* root, caValue* selector, caValue* newValue);

void selector_setup_funcs(Branch* kernel);

} // namespace circa
