// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

#include "common_headers.h"

namespace circa {

struct ForLoopContext
{
    bool discard;

    ForLoopContext() : discard(false) {}
};

Term* get_for_loop_iterator(Term* forTerm);
Term* get_for_loop_modify_list(Term* forTerm);
void setup_for_loop_pre_code(Term* forTerm);
Term* setup_for_loop_iterator(Term* forTerm, const char* name);
void setup_for_loop_post_code(Term* forTerm);

Term* find_enclosing_for_loop(Term* term);

void write_for_loop_bytecode(bytecode::WriteContext* context, Term* forTerm);

CA_FUNCTION(evaluate_for_loop);
void for_loop_assign_registers(Term* term);

} // namespace circa
