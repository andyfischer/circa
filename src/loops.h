// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

const char* for_loop_get_iterator_name(Term* forTerm);
Term* for_loop_find_index(Block* contents);
Term* for_loop_find_output_index(Block* contents);


// Initialize the contents of a for-loop for a new term. 'iteratorType' is the type to use
// for the iterator. If it's NULL then we'll infer a type from the term's input.
Term* start_building_for_loop(Term* forTerm, const char* iteratorName, Type* iteratorType);
void finish_for_loop(Term* forTerm);

Term* find_enclosing_for_loop(Term* term);
Block* find_enclosing_for_loop_contents(Term* term);
bool loop_produces_output_value(Term* forTerm);
bool enclosing_loop_produces_output_value(Term* term);

Block* for_loop_get_zero_block(Block* forContents);
void for_loop_remake_zero_block(Block* forContents);

void start_for_loop(Stack* stack, bool enableLoopOutput);

void finish_while_loop(Term* whileTerm);
void evaluate_unbounded_loop(caStack*);
void evaluate_unbounded_loop_finish(caStack*);

void loop_setup_functions(Block* kernel);

} // namespace circa
