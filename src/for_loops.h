// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#ifndef CIRCA_FOR_LOOPS_INCLUDED
#define CIRCA_FOR_LOOPS_INCLUDED

#include "common_headers.h"

namespace circa {

Branch& get_for_loop_state(Term* forTerm, int index);
Branch& get_for_loop_iteration_state(Term* forTerm, int index);
Term* get_for_loop_iterator(Term* forTerm);
bool& get_for_loop_modify_list(Term* forTerm);
bool& get_for_loop_discard_called(Term* forTerm);
void evaluate_for_loop(Term* forTerm);
void setup_for_loop_pre_code(Term* forTerm);
void setup_for_loop_post_code(Term* forTerm);

Term* find_enclosing_for_loop(Term* term);

} // namespace circa

#endif // CIRCA_FOR_LOOPS_INCLUDED
