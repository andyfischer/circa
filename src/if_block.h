// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

void finish_if_block(Term* ifCall);

int if_block_num_branches(Term* ifCall);
Branch* if_block_get_branch(Term* ifCall, int index);

CA_FUNCTION(evaluate_if_block);

} // namespace circa
