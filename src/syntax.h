// Copyright 2008 Andrew Fischer

#ifndef CIRCA_SOURCE_INCLUDED
#define CIRCA_SOURCE_INCLUDED

namespace circa {

std::string& get_input_syntax_hint(Term* term, int index, std::string const& field);
std::string get_source_of_input(Term* term, int inputIndex);
std::string get_term_source(Term* term);
std::string get_comment_string(Term* term);
bool should_print_term_source_line(Term* term);
std::string get_branch_source(Branch& branch);

}

#endif // CIRCA_SOURCE_INCLUDED
