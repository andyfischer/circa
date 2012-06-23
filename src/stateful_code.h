// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

bool is_function_stateful(Term* func);

Term* find_active_state_container(Branch* branch);
Term* find_or_create_state_container(Branch* branch);

// If the branch has state, this adds a pack_state call that captures state
// values at the current position. If the branch has no state then this
// returns NULL.
Term* branch_add_pack_state(Branch* branch);

bool is_declared_state(Term* term);
void unpack_state(caStack* stack);
void pack_state(caStack* stack);

void get_declared_state(caStack* stack);

void branch_update_existing_pack_state_calls(Branch* branch);

} // namespace circa
