// Copyright 2008 Paul Hodge

#ifndef CIRCA_EVALUATOR_INCLUDED
#define CIRCA_EVALUATOR_INCLUDED

#include "common_headers.h"

namespace circa {

void evaluate_term(Term* term);
void evaluate_branch(Branch& branch, Term* errorListener=NULL);

void error_occured(Term* errorTerm, std::string const& message);

Term* create_term(Branch* branch, Term* function, ReferenceList const& inputs);
void set_inputs(Term* term, ReferenceList const& inputs);
Term* create_value(Term* type);
Term* create_value(Branch* branch, Term* type);
void set_input(Term* term, int index, Term* input);

// Examine 'function' and 'inputs' and returns a result term. A few things
// may happen here:
//  1. We might re-use an existing term
//  2. 'function' might be a type (create an empty instance)
//  3. We might specialize an overloaded function
//  4. Something else that is possibly implemented in the future?
Term* apply_function(Branch& branch, Term* function, ReferenceList const& inputs);

// Look up the given function in branch, and then call the above apply_function
Term* apply_function(Branch& branch,
                     std::string const& functionName, 
                     ReferenceList const& inputs);

// Perform 'apply_function' and then evaluate the result
Term* eval_function(Branch& branch, Term* function, ReferenceList const& inputs);

// Look up the given function in branch, and then do the above 'eval_function'
Term* eval_function(Branch& branch,
                    std::string const& functionName,
                    ReferenceList const& inputs);

void change_function(Term* term, Term* new_function);

void remap_pointers(Term* term, ReferenceMap const& map);
void remap_pointers(Term* term, Term* original, Term* replacement);
void visit_pointers(Term* term, PointerVisitor &visitor);

// Assign the pointer 'pointer' to 'value', where 'owner' is the owner.
// This updates the 'users' field of value (if value is non-NULL),
// and it also updates 'users' on the previous value of 'pointer' (if
// that was non-NULL). 'owner' must not be NULL.
void assign_pointer(Term*& pointer, Term* value, Term* owner);

} // namespace circa

#endif
