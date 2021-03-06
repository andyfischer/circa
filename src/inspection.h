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
bool is_declared_state(Term* term);

bool is_an_unknown_identifier(Term* term);

bool is_major_block(Block* block);
bool is_minor_block(Block* block);
bool is_module(Block* block);

// Find the nearest (parent) block that is a major block. May return the 'block' itself.
Block* find_nearest_major_block(Block* block);
bool is_under_same_major_block(Term* a, Term* b);

// Find the nearest (parent) block that is a compilation unit (such as a module).
Block* find_nearest_compilation_unit(Block* block);

int find_index_of_vararg(Block* block);

// Input & output placeholders
Term* get_input_placeholder(Block* block, int index);

int input_placeholder_index(Term* inputPlaceholder);
int output_placeholder_index(Term* outputPlaceholder);
bool is_input_placeholder(Term* term);
bool is_output_placeholder(Term* term);
Term* find_input_with_name(Block* block, Value* name);

// Other properties on inputs.
bool is_input_meta(Block* block, int index);

Term* term_get_input_placeholder(Term* call, int index);
int term_count_input_placeholders(Term* term);
Term* term_get_output_placeholder(Term* call, int index);
bool term_has_variable_args(Term* term);

// Return a count of 'actual' output terms (includes the term plus any adjacent
// extra_output terms).
int count_actual_output_terms(Term* term);

Term* find_input_placeholder_with_name(Block* block, Value* name);

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
    RawOutputPrefs() : indentLevel(0), showAllIDs(false), showProperties(false) {}
};

void print_block(Block* block, RawOutputPrefs* prefs, Value* out);
void print_term(Term* term, RawOutputPrefs* prefs, Value* out);
void print_term(Term* term, Value* out);

// Convenient overloads for raw format printing
void print_block(Block* block, Value* out);
void print_block_with_properties(Block* block, Value* out);
void get_block_raw(Block* block, Value* out);
void get_term_to_string_extended(Term*, Value* out);
void get_term_to_string_extended_with_props(Term*, Value* out);

void get_short_location(Term* term, Value* str);
void get_source_filename(Term* term, Value* out);
Value* get_parent_module_name(Block* block);

void list_names_that_this_block_rebinds(Block* block, std::vector<std::string> &names);

// Get a list of the set of terms which descend from 'inputs', and which have 'outputs'
// as descendants.
TermList get_involved_terms(TermList inputs, TermList outputs);

typedef bool (*NamedTermVisitor) (Term* term, const char* name, Value* context);
void visit_name_accessible_terms(Term* location, NamedTermVisitor visitor, Value* context);

// Path expressions
void parse_path_expression(const char* expr, Value* valueOut);
Term* find_term_from_path(Block* root, Value* path);
Term* find_term_from_path(Block* root, const char* pathExpr);
Block* find_block_from_path(Block* root, const char* pathExpr);

} // namespace circa
