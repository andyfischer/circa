// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

void closure_redirect_outside_references(Block* block);
void closures_install_functions(Block* kernel);

Block* get_block_of_callable(caValue* value);

} // namespace circa
