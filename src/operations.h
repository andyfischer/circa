// Copyright 2008 Andrew Fischer

#ifndef CIRCA_OPERATIONS_INCLUDED
#define CIRCA_OPERATIONS_INCLUDED

#include "function.h"
#include "term.h"
#include "type.h"
#include "ref_list.h"

namespace circa {

Term* create_term(Branch* branch, Term* function, ReferenceList inputs);
void set_inputs(Term* term, ReferenceList inputs);
Term* create_var(Branch* branch, Term* type);
void set_input(Term* term, int index, Term* input);

// Examine 'function' and 'inputs' and returns a result term. A few things
// may happen here:
//  1. We might re-use an existing term
//  2. 'function' might be a type (create an empty instance)
//  3. We might specialize an overloaded function
//  4. add more stuff here
Term* apply_function(Branch& branch, Term* function, ReferenceList inputs);

// Perform 'apply_function' and then evaluate the result
Term* eval_function(Branch& branch, Term* function, ReferenceList inputs);

// Fetch the const function for this type
Term* get_var_function(Branch& branch, Term* type);

// Return true if the term is a constant
bool is_var(Term* term);

void change_function(Term* term, Term* new_function);

void remap_pointers(Term* term, ReferenceMap const& map);
void remap_pointers(Term* term, Term* original, Term* replacement);

} // namespace circa

#endif
