// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

// Inspection
int if_block_count_cases(Block* block);
Term* if_block_get_case(Block* block, int index);
bool is_case_block(Block* block);
bool is_if_block(Block* block);
Block* get_block_for_case_block(Block* caseBlock);
Term* if_block_get_output_by_name(Block* block, const char* name);

// Building
void if_block_start(Block* block);
Term* if_block_append_case(Block* block, Term* input);
Term* if_block_append_output(Block* block, const char* name);
void if_block_finish_appended_case(Block* block, Term* caseTerm);
void finish_if_block(Term* ifBlock);

} // namespace circa
