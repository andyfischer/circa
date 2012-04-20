// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

int if_block_count_cases(Term* term);
Term* if_block_get_case(Term* term, int index);
Term* if_block_append_case(Term* ifBlock, Term* input);
void if_block_finish_appended_case(Term* ifBlock, Term* caseTerm);
void finish_if_block(Term* ifBlock);
void if_block_post_setup(Term* ifCall);

} // namespace circa
