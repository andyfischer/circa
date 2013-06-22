// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

void closure_redirect_outside_references(Block* block);
void closures_install_functions(Block* kernel);

void closure_block_evaluate(caStack* stack);

bool is_closure(caValue* value);
void set_closure(caValue* value, Block* block, caValue* bindings);
caValue* closure_get_block(caValue* value);
caValue* closure_get_bindings(caValue* value);

} // namespace circa
