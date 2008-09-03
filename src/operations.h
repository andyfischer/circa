#ifndef CIRCA__OPERATIONS__INCLUDED
#define CIRCA__OPERATIONS__INCLUDED

#include "function.h"
#include "term.h"
#include "type.h"

namespace circa {

Term* create_term(Branch* branch, Term* function, TermList inputs);
void initialize_term(Term* term, Term* function, TermList inputs);
void set_inputs(Term* term, TermList inputs);
Term* create_constant(Branch* branch, Term* type);
void unsafe_change_type(Term* term, Term* type);
void change_type(Term* term, Term* type);
void specialize_type(Term* term, Term* type);
void set_input(Term* term, int index, Term* input);
void execute(Term* term);
void execute_branch(Branch* branch);

// Examine 'function' and 'inputs' and returns a result term. A few things
// may happen here:
//  1. We might re-use an existing term
//  2. 'function' might be a type (create an empty instance)
//  3. We might specialize an overloaded function
//  4. add more stuff here
Term* apply_function(Branch* branch, Term* function, TermList inputs);

// Perform 'apply_function' and then execute the result
Term* exec_function(Branch* branch, Term* function, TermList inputs);

// Fetch the const function for this type
Term* get_const_function(Branch* branch, Term* type);

// Return true if the term is a constant
bool is_constant(Term* term);

void change_function(Term* term, Term* new_function);

void dealloc_value(Term* term);

// recycle_value will either call duplicate_value or steal_value, depending
// on heuristics
void recycle_value(Term* source, Term* dest);

void duplicate_value(Term* source, Term* dest);

// Attempt to 'steal' the output value from source. This is more efficient
// than copying, and useful if 1) dest needs a copy of source's value, and
// 2) you don't think that anyone will need source.
//
// Note that (2) is not a hard requirement since we can usually recreate the
// value. But it may be less efficient to do so.
//
// In some situations we are not allowed to steal a value. In these situations,
// calling steal_value is equivalent to calling duplicate_value. These situations include:
//  1) if source is a constant
//  2) ... (possible future situations)
void steal_value(Term* source, Term* dest);

void remap_pointers(Term* term, Term* original, Term* replacement);

void duplicate_branch(Branch* source, Branch* dest);

Term* find_named(Branch* branch, std::string name);

Term* constant_string(Branch* branch, std::string const& s, std::string const& name="");
Term* constant_int(Branch* branch, int i, std::string const& name="");
Term* constant_float(Branch* branch, float f, std::string const& name="");
Term* constant_list(Branch* branch, TermList list, std::string const& name="");

} // namespace circa

#endif
