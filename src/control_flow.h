// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

bool is_exit_point(Term* term);
Symbol term_get_highest_exit_level(Term* term);

// Find the that belongs to the term. Returns NULL if none.
Term* find_trailing_exit_point(Term* term);

// For the given output, find the intermediate value at the given location. The definition
// of "intermediate value" depends on what kind of output it is. For an implicit named output,
// the intermediate value is the nearby term with the same name binding.
Term* find_intermediate_result_for_output(Term* location, Term* output);

// Create or destroy the necessary exit_point calls inside 'block'.
void update_exit_points(Block* block);

void control_flow_setup_funcs(Block* kernel);

} // namespace circa
