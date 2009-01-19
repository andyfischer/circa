// Copyright 2008 Andrew Fischer

#ifndef CIRCA_SOURCE_INCLUDED
#define CIRCA_SOURCE_INCLUDED

namespace circa {

std::string get_term_source(Term* term);
bool should_print_term_source_line(Term* term);
std::string get_branch_source(Branch& branch);

}

#endif // CIRCA_SOURCE_INCLUDED
