// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

#include "common_headers.h"

namespace circa {

void update_if_block_joining_branch(Term* ifCall);
Branch* get_if_condition_block(Term* ifCall, int index);
Branch* get_if_block_else_block(Term* ifCall);
bool if_block_contains_state(Term* ifCall);

CA_FUNCTION(evaluate_if_block);

} // namespace circa
