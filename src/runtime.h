// Copyright 2008 Andrew Fischer

#ifndef CIRCA_RUNTIME_INCLUDED
#define CIRCA_RUNTIME_INCLUDED

#include "common_headers.h"
#include "pointer_visitor.h"

namespace circa {

void evaluate_term(Term* term);
void evaluate_branch(Branch& branch, Term* errorListener=NULL);

// Puts 'errorTerm' into an error state, with the given message.
void error_occured(Term* errorTerm, std::string const& message);

// Create a term with the given function and inputs.
// This function does not do any magic; it (attempts) to use your exact function and 
// exact inputs. You may want to instead use apply_function(), which does some
// helpful magic.
Term* create_term(Branch* branch, Term* function, ReferenceList const& inputs);

void set_input(Term* term, int index, Term* input);
void set_inputs(Term* term, ReferenceList inputs);

void delete_term(Term* term);

// Check the function and inputs of 'user', returns whether they are actually
// using 'usee'.
bool is_actually_using(Term* user, Term* usee);

// Examine 'function' and 'inputs' and returns a result term. A few things
// may happen here:
//  1. We might re-use an existing term
//  2. 'function' might be a type, in which case we create a value of this type.
//  3. We might specialize an overloaded function
//  4. We might coerce inputs to a different type.
Term* apply_function(Branch& branch, Term* function, ReferenceList const& inputs);

// Find the named function in this branch, and then call the above apply_function.
Term* apply_function(Branch& branch,
                     std::string const& functionName, 
                     ReferenceList const& inputs);

// Perform 'apply_function' and then evaluate the result.
Term* eval_function(Branch& branch, Term* function, ReferenceList const& inputs);

// Find the named function in this branch, and then do the above 'eval_function'.
Term* eval_function(Branch& branch,
                    std::string const& functionName,
                    ReferenceList const& inputs);

void change_function(Term* term, Term* new_function);

void remap_pointers(Term* term, ReferenceMap const& map);
void remap_pointers(Term* term, Term* original, Term* replacement);
void visit_pointers(Term* term, PointerVisitor &visitor);


} // namespace circa

#endif
