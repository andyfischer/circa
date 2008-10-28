// Copyright 2008 Andrew Fischer

#ifndef CIRCA_INTROSPECTION_INCLUDED
#define CIRCA_INTROSPECTION_INCLUDED

namespace circa {

void print_term_extended(Term* term, std::ostream &output);
void print_branch_extended(Branch& branch, std::ostream &output);
ReferenceList list_all_pointers(Term* term);
void print_terms(ReferenceList const& list, std::ostream &output);
bool is_equivalent(Term* target, Term* function, ReferenceList const& inputs);
Term* find_equivalent(Term* function, ReferenceList const& inputs);

}

#endif
