// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

void selector_prepend(caValue* selector, caValue* element);

// Write a set_with_selector expression that creates a selector() from 'accessor', and
// assigns 'result' to the subelement. If the accessor has no selector then we simply
// rename 'result' to have 'accessor's name.
Term* write_set_selector_result(Branch* branch, Term* accessor, Term* result);

caValue* get_with_selector(caValue* root, caValue* selector, caValue* error);
void set_with_selector(caValue* root, caValue* selector, caValue* newValue, caValue* error);

void selector_setup_funcs(Branch* kernel);

} // namespace circa
