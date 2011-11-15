// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

// Examine 'function' and 'inputs' and returns a result term.
Term* apply(Branch* branch, Term* function, TermList const& inputs,
    std::string const& name="");

// Find the named function in this branch, and then call the above apply.
Term* apply(Branch* branch, std::string const& functionName, 
                 TermList const& inputs, std::string const& name="");

// Create a duplicate of the given term.
// If 'copyBranches' is false, don't copy branch state. It's assumed that the
// caller will do this. This functionality is used by duplicate_branch
Term* create_duplicate(Branch* branch, Term* original, std::string const& name="",
        bool copyBranches=true);

// Inputs and user lists:
void set_input(Term* term, int index, Term* input);
void set_inputs(Term* term, TermList const& inputs);
void insert_input(Term* term, Term* input);
bool is_actually_using(Term* user, Term* usee);
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
void unsafe_change_type(Term* term, Type* type);
void change_declared_type(Term* term, Type* type);
void respecialize_type(Term* term);
void specialize_type(Term* term, Type* type);

// Rename term, modify the name binding of the owning branch if necessary
void rename(Term* term, std::string const& name);

// Create a new value term with the given type.
Term* create_value(Branch* branch, Type* type, std::string const& name="");
Term* create_value(Branch* branch, std::string const& typeName, std::string const& name="");
Term* create_value(Branch* branch, TaggedValue* initialValue, std::string const& name="");

Term* create_stateful_value(Branch* branch, Type* type, Term* defaultValue,
        std::string const& name);

// Create values with a specified value.
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
Term* create_symbol_value(Branch* branch, TaggedValue* value, std::string const& name="");
Term* duplicate_value(Branch* branch, Term* term);

// In this context, "procure" means "return the existing thing if it already exists, and
// create it if it doesn't exist." Procure functions are idempotent.
Term* procure_value(Branch* branch, Type* type, std::string const& name);

Term* procure_int(Branch* branch, std::string const& name);
Term* procure_float(Branch* branch, std::string const& name);
Term* procure_bool(Branch* branch, std::string const& name);

Term* get_output_placeholder(Branch* branch, int index);

// Fetch the nth output_placeholder for this call. The placeholder term is sometimes
// found inside the function definition, but for 'if' and 'for' blocks the placeholder
// is inside nested contents.
Term* term_get_input_placeholder(Term* call, int index);
Term* term_get_output_placeholder(Term* call, int index);

Term* find_open_state_result(Branch* branch, int position);
Term* find_or_create_open_state_result(Branch* branch, int position);

// Check the term's inputs to see if it's missing an implicit input (such as the state
// input). If one is missing, it will be inserted. This should be called after creating
// a term and updating the input properties.
void check_to_insert_implicit_inputs(Term* term);
bool term_is_state_input(Term* term, int index);
Term* find_state_input(Branch* branch);
Term* find_state_output(Branch* branch);
Term* insert_state_input(Branch* branch);

void set_step(Term* term, float step);
float get_step(Term* term);

// Call the term's postCompile handler, if there is one.
void post_compile_term(Term* term);

// Add a finish_minor_branch() term to this branch, if needed.
void finish_minor_branch(Branch* branch);

void check_to_add_branch_finish_term(Branch* branch, int previousLastTerm);
void update_branch_finish_term(Term* term);
Term* find_last_non_comment_expression(Branch* branch);

bool branch_creates_stack_frame(Branch* branch);
int get_frame_distance(Branch* frame, Term* input);
int get_frame_distance(Term* term, Term* input);

// Input instructions:
void write_stack_input_instruction(Branch* callingFrame, Term* input, TaggedValue* isn);
ListData* write_input_instruction_list(Term* caller, ListData* list);
ListData* write_output_instruction_list(Term* caller, ListData* list);

} // namespace circa
