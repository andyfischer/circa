// Copyright 2008 Andrew Fischer

#ifndef CIRCA_SOURCE_REPRO_INCLUDED
#define CIRCA_SOURCE_REPRO_INCLUDED

// source_repro.h
//
// Various functions for reproducing source text.

namespace circa {

std::string& get_input_syntax_hint(Term* term, int index, std::string const& field);
std::string get_source_of_input(Term* term, int inputIndex);
std::string get_term_source(Term* term);
std::string get_comment_string(Term* term);
bool should_print_term_source_line(Term* term);
std::string get_branch_source(Branch& branch);

}

#endif
