#ifndef CIRCA_EVALUATOR_INCLUDED
#define CIRCA_EVALUATOR_INCLUDED

#include "common_headers.h"

namespace circa {

void evaluate_term(Term* term);
void evaluate_branch(Branch& branch);
Branch* evaluate_file(std::string const& filename);
void error_occured(Term* errorTerm, std::string const& message);

Term* create_term(Branch* branch, Term* function, ReferenceList const& inputs);
void set_inputs(Term* term, ReferenceList const& inputs);
Term* create_var(Branch* branch, Term* type);
void set_input(Term* term, int index, Term* input);

// Call this whenever the term 'pointer' is modified to point at 'pointee'
void register_pointer(Term* pointer, Term* pointee);

// Examine 'function' and 'inputs' and returns a result term. A few things
// may happen here:
//  1. We might re-use an existing term
//  2. 'function' might be a type (create an empty instance)
//  3. We might specialize an overloaded function
//  4. add more stuff here
Term* apply_function(Branch& branch, Term* function, ReferenceList const& inputs);

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

} // namespace circa

#endif
