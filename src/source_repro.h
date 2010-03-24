// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#ifndef CIRCA_SOURCE_REPRO_INCLUDED
#define CIRCA_SOURCE_REPRO_INCLUDED

// source_repro.h
//
// Various functions for reproducing source text.

namespace circa {

int get_first_visible_input_index(Term* term);
std::string const& get_input_syntax_hint(Term* term, int index, std::string const& field);
std::string get_input_syntax_hint_optional(Term* term, int index, std::string const& field,
        std::string const& defaultValue);
void set_input_syntax_hint(Term* term, int index, std::string const& field,
        std::string const& value);
bool has_source_location_defined(Term* term);
std::string get_source_of_input(Term* term, int inputIndex);
bool is_hidden(Term* term);
void prepend_name_binding(Term* term, std::stringstream& out);
std::string get_term_source(Term* term);
std::string get_term_source_default_formatting(Term* term);
std::string get_comment_string(Term* term);
bool should_print_term_source_line(Term* term);

void print_branch_source(std::ostream& output, Term* term);

// deprecated:
std::string get_branch_source(Branch& branch, std::string const& defaultSeparator="\n");

}

#endif
