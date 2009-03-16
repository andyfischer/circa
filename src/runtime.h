// Copyright 2008 Paul Hodge

#ifndef CIRCA_RUNTIME_INCLUDED
#define CIRCA_RUNTIME_INCLUDED

#include "common_headers.h"

namespace circa {

void evaluate_term(Term* term);
void evaluate_branch(Branch& branch, Term* errorListener=NULL);

// Puts 'errorTerm' into an error state, with the given message.
void error_occured(Term* errorTerm, std::string const& message);

// Create a term with the given function and inputs.
// This function does not do any magic; it (attempts) to use your exact function and 
// exact inputs. You may want to instead use apply_function(), which does some
// helpful magic.
Term* create_term(Branch* branch, Term* function, RefList const& inputs);

void set_input(Term* term, int index, Term* input);

// Create a duplicate of the given term
// If 'copyBranches' is false, don't copy branch state. It's assumed that the
// caller will do this. This functionality is used by duplicate_branch
Term* create_duplicate(Branch* branch, Term* source, bool copyBranches=true);

// Examine 'function' and 'inputs' and returns a result term. A few things
// may happen here:
//  1. We might re-use an existing term
//  2. 'function' might be a type, in which case we create a value of this type.
//  3. We might specialize an overloaded function
//  4. We might coerce inputs to a different type.
Term* apply_function(Branch* branch, Term* function, RefList const& inputs, std::string const& name="");

// Find the named function in this branch, and then call the above apply_function.
Term* apply_function(Branch* branch,
                     std::string const& functionName, 
                     RefList const& inputs);

// Perform 'apply_function' and then evaluate the result.
Term* eval_function(Branch& branch, Term* function, RefList const& inputs);

// Find the named function in this branch, and then do the above 'eval_function'.
Term* eval_function(Branch& branch,
                    std::string const& functionName,
                    RefList const& inputs);

void change_function(Term* term, Term* new_function);

void remap_pointers(Term* term, ReferenceMap const& map);
void remap_pointers(Term* term, Term* original, Term* replacement);

} // namespace circa

#endif
