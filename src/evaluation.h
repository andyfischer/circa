// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#ifndef CIRCA_EVALUATION_INCLUDED
#define CIRCA_EVALUATION_INCLUDED

#include "common_headers.h"

namespace circa {

void evaluate_term(Term* term);
void evaluate_branch(Branch& branch, Term* errorListener=NULL);

// Perform 'apply' and then evaluate the result.
Term* apply_and_eval(Branch& branch, Term* function, RefList const& inputs);

// Find the named function in this branch, and then do the above 'apply_and_eval'.
Term* apply_and_eval(Branch& branch,
                    std::string const& functionName,
                    RefList const& inputs);

} // namespace circa

#endif
