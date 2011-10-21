// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <vector>

#include "common_headers.h"

#pragma once

// introspection.h
//
// Functions for doing various queries on code.

namespace circa {

struct RawOutputPrefs
{
    bool showAllIDs;
    bool showProperties;
    RawOutputPrefs() : showAllIDs(false), showProperties(false) {}
};

// Check the function and inputs of 'user', returns whether they are actually
// using 'usee'.
bool is_actually_using(Term* user, Term* usee);

// Set / get whether this term is a 'statement'. This affects source code reproduction.
void set_is_statement(Term* term, bool value);
bool is_statement(Term* term);

bool is_comment(Term* term);
bool is_value(Term* term);
bool is_hidden(Term* term);

bool is_an_unknown_identifier(Term* term);

// Checks if term->nestedContents is a major branch. A 'major' branch has its own stack
// frame when executed.
bool is_major_branch(Term* term);

bool has_an_error_listener(Term* term);

// Format the term's global id as a string that looks like: $ab3
std::string global_id(Term* term);

std::string get_short_local_name(Term* term);

std::string branch_namespace_to_string(Branch* branch);

// Print compiled code in a raw format
void print_branch(std::ostream& out, Branch* branch, RawOutputPrefs* prefs);
void print_term(std::ostream& out, Term* term, RawOutputPrefs* prefs);
void print_term(std::ostream& out, Term* term);

// Convenient overloads for raw format printing
void print_branch(std::ostream& out, Branch* branch);
void print_branch_with_properties(std::ostream& out, Branch* branch);
std::string get_branch_raw(Branch* branch);
std::string get_term_to_string_extended(Term*);
std::string get_term_to_string_extended_with_props(Term*);

// Print a short source-code location for this term.
std::string get_short_location(Term* term);

// Print the source file that this term came from, if any.
std::string get_source_filename(Term* term);

void list_names_that_this_branch_rebinds(Branch* branch, std::vector<std::string> &names);

// Get a list of the set of terms which descend from 'inputs', and which have 'outputs'
// as descendants.
TermList get_involved_terms(TermList inputs, TermList outputs);

typedef bool (*NamedTermVisitor) (Term* term, const char* name, TaggedValue* context);
void visit_name_accessible_terms(Term* location, NamedTermVisitor visitor, TaggedValue* context);

}
