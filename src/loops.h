// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

const char* for_loop_get_iterator_name(Term* forTerm);
Term* for_loop_find_index(Block* contents);

// Initialize the contents of a for-loop for a new term. 'iteratorType' is the type to use
// for the iterator. If it's NULL then we'll infer a type from the term's input.
void start_building_for_loop(Term* forTerm, const char* iteratorName, Type* iteratorType);
void finish_for_loop(Term* forTerm);
void finish_while_loop(Block* block);

Term* find_enclosing_for_loop(Term* term);
Block* find_enclosing_for_loop_contents(Term* term);
bool loop_produces_output_value(Term* forTerm);
bool enclosing_loop_produces_output_value(Term* term);

void list_names_that_must_be_looped(Block* contents, caValue* names);

void loop_add_condition_check(Block* caseBlock, Term* condition);
Term* loop_find_condition_check(Block* block);
Term* loop_find_condition(Block* block);

} // namespace circa
