// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

// introspection.h
//
// Functions for doing various queries on code.

namespace circa {

bool is_value(Term* term);

// Check the function and inputs of 'user', returns whether they are actually
// using 'usee'.
bool is_actually_using(Term* user, Term* usee);

// Set / get whether this term is a 'statement'. This affects source code reproduction.
void set_is_statement(Term* term, bool value);
bool is_statement(Term* term);

// Format the term's global id as a string that looks like: $ab3
std::string format_global_id(Term* term);

std::string get_short_local_name(Term* term);
void print_term_raw_string(std::ostream& out, Term* term);
void print_term_raw_string_with_properties(std::ostream& out, Term* term);
std::string term_to_raw_string(Term* term);
std::string term_to_raw_string_with_properties(Term* term);
std::string branch_namespace_to_string(Branch& branch);
void print_branch_raw(std::ostream& out, Branch& branch);
void print_branch_raw_with_properties(std::ostream& out, Branch& branch);
std::string get_branch_raw(Branch& branch);

// Print a short source-code location for this term.
std::string get_short_location(Term* term);

// Print the source file that this term came from, if any.
std::string get_source_filename(Term* term);

RefList get_influencing_values(Term* term);
void list_names_that_this_branch_rebinds(Branch& branch, std::vector<std::string> &names);

// Get a list of the set of terms which descend from 'inputs', and which have 'outputs'
// as descendants.
RefList get_involved_terms(RefList inputs, RefList outputs);

void print_term_to_string_extended(std::ostream& out, Term* term);

std::string get_term_to_string_extended(Term*);

}
