// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

#include "common_headers.h"

namespace circa {

#ifndef BYTECODE
bool for_loop_has_state(Term* forTerm);
TaggedValue* get_for_loop_iteration_state(Term* forTerm, int index);
#endif
Term* get_for_loop_iterator(Term* forTerm);
Term* get_for_loop_modify_list(Term* forTerm);
#ifndef BYTECODE
Term* get_for_loop_discard_called(Term* forTerm);
Ref& get_for_loop_state_type(Term* forTerm);
#endif
#ifndef BYTECODE
CA_FUNCTION(evaluate_for_loop);
#endif
void setup_for_loop_pre_code(Term* forTerm);
Term* setup_for_loop_iterator(Term* forTerm, const char* name);
void setup_for_loop_post_code(Term* forTerm);

Term* find_enclosing_for_loop(Term* term);

void write_for_loop_bytecode(bytecode::WriteContext* context, Term* forTerm);

} // namespace circa
