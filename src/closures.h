// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

void closure_redirect_outside_references(Block* block);
void closures_install_functions(Block* kernel);

void closure_block_evaluate(caStack* stack);

Block* get_block_of_callable(caValue* value);

} // namespace circa
