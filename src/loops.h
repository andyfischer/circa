// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

const char* for_loop_get_iterator_name(Term* forTerm);
Term* for_loop_find_index(Branch* contents);

// Initialize the contents of a for-loop for a new term. 'iteratorType' is the type to use
// for the iterator. If it's NULL then we'll infer a type from the term's input.
Term* start_building_for_loop(Term* forTerm, const char* iteratorName, Type* iteratorType);
void finish_for_loop(Term* forTerm);

Term* find_enclosing_for_loop(Term* term);
Branch* find_enclosing_for_loop_contents(Term* term);

bool is_for_loop(Branch* branch);
Branch* for_loop_get_zero_branch(Branch* forContents);
void for_loop_remake_zero_branch(Branch* forContents);

CA_FUNCTION(start_for_loop);
void for_loop_finish_iteration(Stack* context);

void finish_while_loop(Term* whileTerm);
CA_FUNCTION(evaluate_unbounded_loop);
CA_FUNCTION(evaluate_unbounded_loop_finish);

} // namespace circa
