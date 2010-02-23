// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#ifndef CIRCA__IF_BLOCK__INCLUDED
#define CIRCA__IF_BLOCK__INCLUDED

#include "common_headers.h"

namespace circa {

void update_if_block_joining_branch(Term* ifCall);
Branch* get_if_condition_block(Term* ifCall, int index);
Branch* get_if_block_else_block(Term* ifCall);
Branch* get_if_block_state(Term* ifCall);
bool if_block_contains_state(Term* ifCall);

void evaluate_if_block(EvalContext*, Term* caller);

} // namespace circa

#endif
