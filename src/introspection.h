// Copyright 2008 Paul Hodge

#ifndef CIRCA_INTROSPECTION_INCLUDED
#define CIRCA_INTROSPECTION_INCLUDED

// introspection.h
//
// Functions for doing various queries on code. These functions should be mostly
// side-effect free.

namespace circa {

bool is_value(Term* term);

// Check the function and inputs of 'user', returns whether they are actually
// using 'usee'.
bool is_actually_using(Term* user, Term* usee);

void set_is_statement(Term* term, bool value);
bool is_statement(Term* term);

std::string format_global_id(Term* term);
std::string get_short_local_name(Term* term);
std::string term_to_raw_string(Term* term);
std::string branch_namespace_to_string(Branch& branch);
std::string branch_to_string_raw(Branch& branch);
std::string branch_to_string_raw_with_properties(Branch& branch);

std::string get_short_location(Term* term);
std::string get_source_filename(Term* term);
RefList get_influencing_values(Term* term);
void list_names_that_this_branch_rebinds(Branch& branch, std::vector<std::string> &names);

// Get a list of the set of terms which descent from 'inputs', and which have 'outputs'
// as descendants.
RefList get_involved_terms(RefList inputs, RefList outputs);

}

#endif
