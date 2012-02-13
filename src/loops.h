// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

Term* get_for_loop_iterator(Term* forTerm);
const char* for_loop_get_iterator_name(Term* forTerm);
Term* for_loop_find_index(Branch* contents);

Term* start_building_for_loop(Term* forTerm, const char* iteratorName);
void finish_for_loop(Term* forTerm);

Term* find_enclosing_for_loop(Term* term);
Branch* find_enclosing_for_loop_contents(Term* term);

CA_FUNCTION(evaluate_for_loop);
CA_FUNCTION(evaluate_loop_output);

void finish_while_loop(Term* whileTerm);
CA_FUNCTION(evaluate_unbounded_loop);
CA_FUNCTION(evaluate_unbounded_loop_finish);

} // namespace circa
