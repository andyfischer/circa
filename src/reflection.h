// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

void reflection_install_functions(Block* kernel);

void update_all_code_references_in_value(caValue* value, Block* oldBlock, Block* newBlock);

} // namespace circa
