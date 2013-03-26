// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"
#include "names.h"

namespace circa {

// Examine 'function' and 'inputs' and returns a result term.
Term* apply(Block* block, Term* function, TermList const& inputs, caValue* name = NULL);

// Find the named function in this block, and then call the above apply.
Term* apply(Block* block, const char* functionName, TermList const& inputs,
        caValue* name = NULL);

Term* apply(Block* block, Term* function, TermList const& inputs, const char* name);

// Create a duplicate of the given term. Doesn't duplicate nestedContents.
Term* create_duplicate(Block* block, Term* original, caValue* name);

// Inputs and user lists:
void set_input(Term* term, int index, Term* input);
void set_inputs(Term* term, TermList const& inputs);
void insert_input(Term* term, Term* input);
void insert_input(Term* term, int index, Term* input);
void append_user(Term* user, Term* usee);
void possibly_prune_user_list(Term* user, Term* usee);

// This finds all the terms which have this term as a user, and removes it from
// their user list. This creates a temporary inconsistency (because the term is
// still technically using those things) but it's appropriate when you want to delete
// this term.
void remove_from_any_user_lists(Term* term);

// This checks every user of this term, and removes it from their input lists.
void clear_from_dependencies_of_users(Term* term);

// Change a term's function
void change_function(Term* term, Term* function);

// Change a term's declared type
void change_declared_type(Term* term, Type* type);
void respecialize_type(Term* term);

// Rename term, modify the name binding of the owning block if necessary
void rename(Term* term, caValue* name);
void rename(Term* term, const char* name);

// Create a new value term with the given type.
Term* create_value(Block* block, Type* type, const char* name=NULL);
Term* create_value(Block* block, const char* typeName, const char* name=NULL);
Term* create_value(Block* block, caValue* initialValue, const char* name=NULL);

Term* create_string(Block* block, const char* s, const char* name=NULL);
Term* create_int(Block* block, int i, const char* name=NULL);
Term* create_float(Block* block, float f, const char* name=NULL);
Term* create_bool(Block* block, bool b, const char* name=NULL);
Term* create_void(Block* block, const char* name=NULL);
Term* create_list(Block* block, const char* name=NULL);
Block* create_block(Block* owner, const char* name);
Block* find_or_create_block(Block* owner, const char* name);
Block* create_namespace(Block*, const char* name);
Block* create_block_unevaluated(Block* owner, const char* name);
Term* create_type(Block* block, const char* name);
Term* create_type_value(Block* block, Type* value, const char* name=NULL);
Term* create_symbol_value(Block* block, int value, const char* name=NULL);
Term* duplicate_value(Block* block, Term* term);

// Add an input_placeholder() term after the existing placeholders.
Term* append_input_placeholder(Block* block);
Term* append_output_placeholder(Block* block, Term* result);
Term* append_output_placeholder_with_description(Block* block, caValue* description);
Term* prepend_output_placeholder(Block* block, Term* result);
Term* insert_output_placeholder(Block* block, Term* result, int location);
Term* append_state_input(Block* block);
Term* append_state_output(Block* block);

void get_input_description(Term* output, caValue* result);
Term* find_output_placeholder_with_name(Block* block, caValue* name);
Term* find_output_from_description(Block* block, caValue* description);
void get_output_description(Term* output, caValue* result);

// Stateful inputs & outputs
bool term_is_state_input(Term* term, int index);
Term* find_state_input(Block* block);
bool has_state_input(Block* block);
Term* find_state_output(Block* block);
bool has_state_output(Block* block);
bool is_state_input(Term* placeholder);
bool is_state_output(Term* placeholder);

// For the given output, find the intermediate value at the given location. The definition
// of "intermediate value" depends on what kind of output it is. For an implicit named output,
// the intermediate value is the nearby term with the same name binding.
Term* find_intermediate_result_for_output(Term* location, Term* output);

int count_anonymous_outputs(Block* block);

// Extra outputs
void update_extra_outputs(Term* term);

Term* find_open_state_result(Term* location);
Term* find_open_state_result(Block* block);

// Check the term's inputs to see if it's missing an implicit input (such as the state
// input). If one is missing, it will be inserted. This should be called after creating
// a term and updating the input properties.
void check_to_insert_implicit_inputs(Term* term);
void update_implicit_pack_call(Term* term);

void set_step(Term* term, float step);
float get_step(Term* term);

// Set a block as being 'in progress', meaning that we are actively making changes to it.
void block_start_changes(Block* block);

// Set the block as no longer 'in progress', perform any final cleanup actions.
void block_finish_changes(Block* block);

// Code modification
Term* find_user_with_function(Term* term, const char* funcName);
Term* apply_before(Term* existing, Term* function, int input);
Term* apply_after(Term* existing, Term* function);
void move_before(Term* movee, Term* pivot);
void move_after(Term* movee, Term* position); void move_after_inputs(Term* term);
void move_before_outputs(Term* term);
void move_before_final_terms(Term* term);
void move_to_index(Term* term, int index);
void transfer_users(Term* from, Term* to);

void input_placeholders_to_list(Block* block, TermList* list);
void list_outer_pointers(Block* block, TermList* list);
int find_input_index_for_pointer(Term* call, Term* input);
void check_to_add_primary_output_placeholder(Block* block);
void check_to_add_state_output_placeholder(Block* block);

// Refactoring

// Modify term so that it has the given function and inputs.
void rewrite(Term* term, Term* function, TermList const& _inputs);

// Make sure that block[index] is a value with the given type. If that term exists and
// has a different function or type, then change it. If the block doesn't have that
// index, then add NULL terms until it does.
void rewrite_as_value(Block* block, int index, Type* type);

// Calls erase_term, and will also shuffle the terms inside the owning block to
// fill in the empty index.
void remove_term(Term* term);

void remap_pointers_quick(Term* term, Term* old, Term* newTerm);
void remap_pointers_quick(Block* block, Term* old, Term* newTerm);
void remap_pointers(Term* term, TermMap const& map);
void remap_pointers(Term* term, Term* original, Term* replacement);
void remap_pointers(Block* block, Term* original, Term* replacement);

bool term_is_nested_in_block(Term* term, Block* block);

// Look through the nexted contents of 'term', and find any term references to outer
// terms (terms outside this block). For every outer reference, add an input to
// 'term' and repoint the referencing terms to use an input placeholder.
void create_inputs_for_outer_references(Term* term);

// Preceding term in the same block (may be NULL).
Term* preceding_term(Term* term);

// Preceding term in the same block, or in a parent block. Only searches upwards
// within minor blocks.
Term* preceding_term_recr_minor(Term* term);

// Following term in the same block (may be NULL).
Term* following_term(Term* term);

} // namespace circa
