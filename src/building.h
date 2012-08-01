// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"
#include "names.h"

namespace circa {

// Examine 'function' and 'inputs' and returns a result term.
Term* apply(Branch* branch, Term* function, TermList const& inputs,
        Name name = name_None);

// Find the named function in this branch, and then call the above apply.
Term* apply(Branch* branch, std::string const& functionName,
                 TermList const& inputs, Name name = name_None);

// Create a duplicate of the given term. Doesn't duplicate nestedContents.
Term* create_duplicate(Branch* branch, Term* original, std::string const& name="");

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

// Rename term, modify the name binding of the owning branch if necessary
void rename(Term* term, Name name);

// Create a new value term with the given type.
Term* create_value(Branch* branch, Type* type, std::string const& name="");
Term* create_value(Branch* branch, std::string const& typeName, std::string const& name="");
Term* create_value(Branch* branch, caValue* initialValue, std::string const& name="");

Term* create_string(Branch* branch, std::string const& s, std::string const& name="");
Term* create_int(Branch* branch, int i, std::string const& name="");
Term* create_float(Branch* branch, float f, std::string const& name="");
Term* create_bool(Branch* branch, bool b, std::string const& name="");
Term* create_void(Branch* branch, std::string const& name="");
Term* create_list(Branch* branch, std::string const& name="");
Branch* create_branch(Branch* owner, std::string const& name="");
Branch* create_namespace(Branch*, std::string const& name);
Branch* create_branch_unevaluated(Branch* owner, const char* name);
Term* create_type(Branch* branch, std::string name="");
Term* create_type_value(Branch* branch, Type* value, std::string const& name="");
Term* create_symbol_value(Branch* branch, int value, std::string const& name="");
Term* duplicate_value(Branch* branch, Term* term);

// Add an input_placeholder() term after the existing placeholders.
Term* append_input_placeholder(Branch* branch);
Term* append_output_placeholder(Branch* branch, Term* result);
Term* prepend_output_placeholder(Branch* branch, Term* result);

Branch* term_get_function_details(Term* call);

// Extra outputs
Term* get_output_term(Term* term, int index);
void update_extra_outputs(Term* term);
Term* get_extra_output(Term* term, int index);
Term* find_extra_output_for_state(Term* term);

// Fetch the nth output_placeholder for this call. The placeholder term is sometimes
// found inside the function definition, but for 'if' and 'for' blocks the placeholder
// is inside nested contents.
Term* term_get_input_placeholder(Term* call, int index);
int term_count_input_placeholders(Term* term);
Term* term_get_output_placeholder(Term* call, int index);
bool term_has_variable_args(Term* term);

Term* find_open_state_result(Branch* branch, int position);
Term* find_open_state_result(Term* location);

// Check the term's inputs to see if it's missing an implicit input (such as the state
// input). If one is missing, it will be inserted. This should be called after creating
// a term and updating the input properties.
void check_to_insert_implicit_inputs(Term* term);
void update_implicit_pack_call(Term* term);
bool term_is_state_input(Term* term, int index);
Term* find_state_input(Branch* branch);
bool has_state_input(Branch* branch);
Term* find_state_output(Branch* branch);
bool has_state_output(Branch* branch);
Term* append_state_input(Branch* branch);
Term* append_state_output(Branch* branch);
bool is_state_input(Term* placeholder);
bool is_state_output(Term* placeholder);

// Search upwards starting at 'term', and returns the parent (or the term itself) found
// in 'branch'. Returns NULL if not found.
Term* find_parent_term_in_branch(Term* term, Branch* branch);

void set_step(Term* term, float step);
float get_step(Term* term);

// Set a branch as being 'in progress', meaning that we are actively making changes to it.
void branch_start_changes(Branch* branch);

// Set the branch as no longer 'in progress', perform any final cleanup actions.
void branch_finish_changes(Branch* branch);

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

void input_placeholders_to_list(Branch* branch, TermList* list);
void list_outer_pointers(Branch* branch, TermList* list);
void expand_variadic_inputs_for_call(Branch* branch, Term* call);
int find_input_index_for_pointer(Term* call, Term* input);
void check_to_add_primary_output_placeholder(Branch* branch);
void check_to_add_state_output_placeholder(Branch* branch);
Term* find_intermediate_result_for_output(Term* location, Term* output);

// Refactoring

// Modify term so that it has the given function and inputs.
void rewrite(Term* term, Term* function, TermList const& _inputs);

// Make sure that branch[index] is a value with the given type. If that term exists and
// has a different function or type, then change it. If the branch doesn't have that
// index, then add NULL terms until it does.
void rewrite_as_value(Branch* branch, int index, Type* type);

// Calls erase_term, and will also shuffle the terms inside the owning branch to
// fill in the empty index.
void remove_term(Term* term);

void remap_pointers_quick(Term* term, Term* old, Term* newTerm);
void remap_pointers_quick(Branch* branch, Term* old, Term* newTerm);
void remap_pointers(Term* term, TermMap const& map);
void remap_pointers(Term* term, Term* original, Term* replacement);
void remap_pointers(Branch* branch, Term* original, Term* replacement);

// Examine the 'accessor' term, and walk upwards to construct a selector() expression.
// Each get_index or get_field expression corresponds to a selector element. This
// function will return NULL if there are no selector elements.
Term* write_selector_for_accessor_expression(Branch* branch, Term* accessor, Term** headPtr);


// Look through the nexted contents of 'term', and find any term references to outer
// terms (terms outside this branch). For every outer reference, add an input to
// 'term' and repoint the referencing terms to use an input placeholder.
void create_inputs_for_outer_references(Term* term);

} // namespace circa
