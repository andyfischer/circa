// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

int if_block_count_cases(Term* term);
Term* if_block_get_case(Term* term, int index);
void finish_if_block(Term* ifCall);
CA_FUNCTION(evaluate_if_block);

} // namespace circa
