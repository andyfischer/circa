// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

// Inspection
int if_block_count_cases(Block* block);
int case_block_get_index(Block* block);
Term* case_find_condition(Block* block);
Term* case_find_condition_check(Block* block);
Block* get_case_block(Block* block, int index);
bool is_case_block(Block* block);
bool is_switch_block(Block* block);
Block* get_block_for_case_block(Block* caseBlock);
Term* if_block_get_output_by_name(Block* block, const char* name);

// Building
void if_block_start(Block* block);
Term* if_block_append_case(Block* block);
void case_add_condition_check(Block* caseBlock, Term* condition);
Term* if_block_append_output(Block* block, Value* description);
void if_block_finish_appended_case(Block* block, Term* caseTerm);
void finish_if_block(Term* ifBlock);
void switch_block_post_compile(Term* term);

} // namespace circa
