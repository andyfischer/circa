// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include <vector>

#include "common_headers.h"

#pragma once

// inspection.h
//
// Pure functions for inspecting and performing queries on compiled code.

namespace circa {

// Check the function and inputs of 'user', returns whether they are actually
// using 'usee'.
bool is_actually_using(Term* user, Term* usee);

// Return the static type of 'term'.
Type* declared_type(Term* term);
Term* declared_type_term(Term* term);

// Set / get whether this term is a 'statement'. This affects source code reproduction.
void set_is_statement(Term* term, bool value);
bool is_statement(Term* term);

// Various queries on a call site.
bool is_comment(Term* term);
bool is_empty_comment(Term* term);
bool is_value(Term* term);
bool is_hidden(Term* term);
bool has_empty_name(Term* term);
bool is_copying_call(Term* term);
bool is_dynamic_func_call(Term* term);
bool is_declared_state(Term* term);

bool is_an_unknown_identifier(Term* term);

// Checks if term->nestedContents is a major block.
bool is_major_block(Term* term);
bool is_major_block(Block* block);
bool is_minor_block(Block* block);
bool is_module(Block* block);

// Find the nearest (parent) block that is a major block. May return the 'block' itself.
Block* find_nearest_major_block(Block* block);
bool is_under_same_major_block(Term* a, Term* b);

// Find the nearest (parent) block that is a compilation unit (such as a module).
Block* find_nearest_compilation_unit(Block* block);

bool has_variable_args(Block* block);
int find_index_of_vararg(Block* block);

// Input & output placeholders
Term* get_input_placeholder(Block* block, int index);

// Get the input placeholder that corresponds with a call-site index. This takes varargs
// into account.
Term* get_effective_input_placeholder(Block* block, int inputIndex);

Term* get_output_placeholder(Block* block, int index);
int count_input_placeholders(Block* block);
int count_output_placeholders(Block* block);
int input_placeholder_index(Term* inputPlaceholder);
int output_placeholder_index(Term* outputPlaceholder);
bool is_input_placeholder(Term* term);
bool is_output_placeholder(Term* term);
Term* find_input_with_name(Block* block, Value* name);

// Other properties on inputs.
bool is_input_meta(Block* block, int index);

// Accessors that use a term's 'details' block, which may be the nested block,
// or it might be the function's block, depending on the function.
Block* term_get_function_details(Term* call);
Term* term_get_input_placeholder(Term* call, int index);
int term_count_input_placeholders(Term* term);
Term* term_get_output_placeholder(Term* call, int index);
bool term_has_variable_args(Term* term);

// Return a count of 'actual' output terms (includes the term plus any adjacent
// extra_output terms).
int count_actual_output_terms(Term* term);

Term* find_input_placeholder_with_name(Block* block, caValue* name);

Term* find_expression_for_implicit_output(Block* block);
Term* find_term_with_function(Block* block, Term* func);
Term* find_input_with_function(Term* target, Term* func);
Term* find_user_with_function(Term* target, Term* func);

bool has_an_error_listener(Term* term);

// Format the term's global id as a string that looks like: $ab3
std::string global_id(Term* term);

std::string get_short_local_name(Term* term);

std::string block_namespace_to_string(Block* block);

// Print compiled code in a raw format
struct RawOutputPrefs
{
    int indentLevel;
    bool showAllIDs;
    bool showProperties;
    bool showBytecode;
    RawOutputPrefs() : indentLevel(0), showAllIDs(false), showProperties(false),
        showBytecode(false) {}
};


void print_block(Block* block, RawOutputPrefs* prefs, caValue* out, Stack* stack=NULL);
void print_term(Term* term, RawOutputPrefs* prefs, caValue* out);
void print_term(Term* term, caValue* out);

// Convenient overloads for raw format printing
void print_block(Block* block, caValue* out);
void print_block_with_properties(Block* block, caValue* out);
void get_block_raw(Block* block, caValue* out);
void get_term_to_string_extended(Term*, caValue* out);
void get_term_to_string_extended_with_props(Term*, caValue* out);

// Print a short source-code location for this term.
void get_short_location(Term* term, caValue* str);

// Print the source file that this term came from, if any.
std::string get_source_filename(Term* term);
caValue* get_parent_module_name(Block* block);

void list_names_that_this_block_rebinds(Block* block, std::vector<std::string> &names);

// Get a list of the set of terms which descend from 'inputs', and which have 'outputs'
// as descendants.
TermList get_involved_terms(TermList inputs, TermList outputs);

typedef bool (*NamedTermVisitor) (Term* term, const char* name, caValue* context);
void visit_name_accessible_terms(Term* location, NamedTermVisitor visitor, caValue* context);

// Path expressions
void parse_path_expression(const char* expr, caValue* valueOut);
Term* find_term_from_path(Block* root, caValue* path);
Term* find_term_from_path(Block* root, const char* pathExpr);
Block* find_block_from_path(Block* root, const char* pathExpr);

} // namespace circa
