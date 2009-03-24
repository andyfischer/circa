// Copyright 2008 Paul Hodge

#ifndef CIRCA_RUNTIME_INCLUDED
#define CIRCA_RUNTIME_INCLUDED

#include "common_headers.h"

namespace circa {

void evaluate_term(Term* term);
void evaluate_branch(Branch& branch, Term* errorListener=NULL);

// Puts 'errorTerm' into an error state, with the given message.
void error_occured(Term* errorTerm, std::string const& message);

// Perform 'apply' and then evaluate the result.
Term* eval_function(Branch& branch, Term* function, RefList const& inputs);

// Find the named function in this branch, and then do the above 'eval_function'.
Term* eval_function(Branch& branch,
                    std::string const& functionName,
                    RefList const& inputs);

} // namespace circa

#endif
