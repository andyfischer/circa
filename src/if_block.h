// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

void finish_if_block_minor_branch(Term* branch);
void update_if_block_joining_branch(Term* ifCall);
Branch* get_if_block_joining_branch(Term* ifCall);

CA_FUNCTION(evaluate_if_block);

} // namespace circa
