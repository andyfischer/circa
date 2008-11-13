// Copyright 2008 Paul Hodge

#ifndef CIRCA_INTROSPECTION_INCLUDED
#define CIRCA_INTROSPECTION_INCLUDED

namespace circa {

std::string get_short_local_name(Term* term);
void print_term_extended(Term* term, std::ostream &output);
void print_branch_extended(Branch& branch, std::ostream &output);
ReferenceList list_all_pointers(Term* term);
void print_terms(ReferenceList const& list, std::ostream &output);
bool is_equivalent(Term* target, Term* function, ReferenceList const& inputs);
Term* find_equivalent(Term* function, ReferenceList const& inputs);
void print_runtime_errors(Branch& branch, std::ostream& output);
bool has_compile_errors(Branch& branch);
std::vector<std::string> get_compile_errors(Branch& branch);
void print_compile_errors(Branch& branch, std::ostream& output);

}

#endif
