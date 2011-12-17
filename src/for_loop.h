// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

struct ForLoopContext
{
    bool discard;
    bool breakCalled;
    bool continueCalled;

    ForLoopContext() : discard(false), breakCalled(false), continueCalled(false) {}
};

Term* get_for_loop_iterator(Term* forTerm);
const char* for_loop_get_iterator_name(Term* forTerm);
bool for_loop_modifies_list(Term* forTerm);
Term* for_loop_find_index(Branch* contents);

Term* start_building_for_loop(Term* forTerm, const char* iteratorName);
void finish_for_loop(Term* forTerm);

Term* find_enclosing_for_loop(Term* term);
Branch* find_enclosing_for_loop_contents(Term* term);

void for_loop_update_output_index(Term* forTerm);

CA_FUNCTION(evaluate_for_loop);
CA_FUNCTION(evaluate_loop_output);

} // namespace circa
