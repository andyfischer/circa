// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

bool is_exit_point(Term* term);
Symbol term_get_highest_exit_level(Term* term);
Block* find_block_that_exit_point_will_reach(Term* term);

// Find the that belongs to the term. Returns NULL if none.
Term* find_trailing_exit_point(Term* term);

// For the given output, find the intermediate value at the given location. The definition
// of "intermediate value" depends on what kind of output it is. For an implicit named output,
// the intermediate value is the nearby term with the same name binding.
Term* find_intermediate_result_for_output(Term* location, Term* output);

void update_for_control_flow(Block* block);

void control_flow_setup_funcs(Block* kernel);

} // namespace circa
