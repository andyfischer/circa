// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

// Inspection
int if_block_count_cases(Branch* block);
Term* if_block_get_case(Branch* block, int index);
bool is_case_branch(Branch* branch);
bool is_if_block(Branch* branch);
Branch* get_block_for_case_branch(Branch* caseBranch);
Term* if_block_get_output_by_name(Branch* block, const char* name);

// Building
void if_block_start(Branch* block);
Term* if_block_append_case(Branch* block, Term* input);
Term* if_block_append_output(Branch* block, const char* name);
void if_block_finish_appended_case(Branch* block, Term* caseTerm);
void finish_if_block(Term* ifBlock);

} // namespace circa
