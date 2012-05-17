// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

int if_block_count_cases(Term* term);
Term* if_block_get_case(Term* term, int index);
void if_block_start(Branch* block);
Term* if_block_append_case(Branch* block, Term* input);
bool is_case_branch(Branch* branch);
Branch* get_block_for_case_branch(Branch* caseBranch);
Term* if_block_get_output_by_name(Branch* block, const char* name);
Term* if_block_append_output(Branch* block, const char* name);
void if_block_finish_appended_case(Term* ifBlock, Term* caseTerm);
void finish_if_block(Term* ifBlock);
void if_block_post_setup(Term* ifCall);

} // namespace circa
