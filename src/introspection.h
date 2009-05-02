// Copyright 2008 Paul Hodge

#ifndef CIRCA_INTROSPECTION_INCLUDED
#define CIRCA_INTROSPECTION_INCLUDED

// introspection.h
//
// Functions for doing various queries on code. These functions should be mostly
// side-effect free.

namespace circa {

bool is_value(Term* term);
bool is_stateful(Term* term);

// Check the function and inputs of 'user', returns whether they are actually
// using 'usee'.
bool is_actually_using(Term* user, Term* usee);

bool has_inner_branch(Term* term);
Branch* get_inner_branch(Term* term);
std::string get_short_local_name(Term* term);
std::string term_to_raw_string(Term* term);
std::string branch_namespace_to_string(Branch& branch);
std::string branch_to_string_raw(Branch& branch);
bool is_equivalent(Term* target, Term* function, RefList const& inputs);
Term* find_equivalent(Branch& branch, Term* function, RefList const& inputs);
void print_runtime_errors(Branch& branch, std::ostream& output);
int count_compile_errors(Branch& branch);
std::string get_short_location(Term* term);
std::string get_source_filename(Term* term);
void print_compile_errors(Branch& branch, std::ostream& output);
RefList get_influencing_values(Term* term);

// Get a list of the set of terms which descent from 'inputs', and which have 'outputs'
// as descendants.
RefList get_involved_terms(RefList inputs, RefList outputs);

}

#endif
