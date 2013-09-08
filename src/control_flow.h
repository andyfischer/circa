// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

bool is_exit_point(Term* term);
bool is_conditional_exit_point(Term* term);
Symbol term_get_highest_exit_level(Term* term);
Block* find_block_that_exit_point_will_reach(Term* term);
bool has_escaping_control_flow(Term* term);

void update_derived_inputs_for_exit_point(Term* term);
void update_for_control_flow(Block* block);

void control_flow_setup_funcs(Block* kernel);

} // namespace circa
