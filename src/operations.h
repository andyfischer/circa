// Copyright 2008 Paul Hodge

#ifndef CIRCA_OPERATIONS_INCLUDED
#define CIRCA_OPERATIONS_INCLUDED

#include "function.h"
#include "term.h"
#include "type.h"
#include "ref_list.h"

namespace circa {

Term* create_term(Branch* branch, Term* function, ReferenceList inputs);
void initialize_term(Term* term, Term* function, ReferenceList inputs);
void set_inputs(Term* term, ReferenceList inputs);
Term* create_constant(Branch* branch, Term* type);
void set_input(Term* term, int index, Term* input);

// Examine 'function' and 'inputs' and returns a result term. A few things
// may happen here:
//  1. We might re-use an existing term
//  2. 'function' might be a type (create an empty instance)
//  3. We might specialize an overloaded function
//  4. add more stuff here
Term* apply_function(Branch* branch, Term* function, ReferenceList inputs);

// Perform 'apply_function' and then evaluate the result
Term* eval_function(Branch* branch, Term* function, ReferenceList inputs);

// Fetch the const function for this type
Term* get_const_function(Branch* branch, Term* type);

// Return true if the term is a constant
bool is_constant(Term* term);

void change_function(Term* term, Term* new_function);

void dealloc_value(Term* term);

void remap_pointers(Term* term, ReferenceMap const& map);
void remap_pointers(Term* term, Term* original, Term* replacement);

void duplicate_branch(Branch* source, Branch* dest);

Term* constant_string(Branch* branch, std::string const& s, std::string const& name="");
Term* constant_int(Branch* branch, int i, std::string const& name="");
Term* constant_float(Branch* branch, float f, std::string const& name="");
Term* constant_list(Branch* branch, ReferenceList list, std::string const& name="");

void error_occured(Term* errorTerm, std::string const& message);

} // namespace circa

#endif
