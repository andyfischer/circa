// Copyright 2008 Andrew Fischer

#ifndef CIRCA_RUNTIME_INCLUDED
#define CIRCA_RUNTIME_INCLUDED

#include "common_headers.h"

namespace circa {

Term* get_global(std::string name);

void evaluate_term(Term* term);
void evaluate_branch(Branch& branch, Term* errorListener=NULL);

// Puts 'errorTerm' into an error state, with the given message.
void error_occured(Term* errorTerm, std::string const& message);

// Perform 'apply' and then evaluate the result.
Term* apply_and_eval(Branch* branch, Term* function, RefList const& inputs);

// Find the named function in this branch, and then do the above 'apply_and_eval'.
Term* apply_and_eval(Branch* branch,
                    std::string const& functionName,
                    RefList const& inputs);

int& as_int(Term*);
float& as_float(Term*);
bool& as_bool(Term*);
std::string& as_string(Term*);
Ref& deref(Term*);
bool is_string(Term* term);

} // namespace circa

#endif
