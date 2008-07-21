
#include "codeunit.h"
#include "function.h"
#include "term.h"
#include "type.h"


Term* create_term(Term* function, TermList inputs);
void initialize_term(Term* term, Term* function);
Term* create_constant(Term* type);
void change_type(Term* term, Term* type);
void specialize_type(Term* term, Term* type);
void set_input(Term* term, int index, Term* input);
void set_inputs(Term* term, TermList inputs);

// Examine 'function' and 'inputs' and returns a result term. A few things
// may happen here:
//  1. We might re-use an existing term
//  2. 'function' might be a type
//  3. We might specialize an overloaded function
//  4. add more stuff here
Term* apply_function(Term* function, TermList inputs);

// Fetch the const function for this type
Term* get_const_function(Term* type);

Term* quick_create_type(CodeUnit* code, string name, Type::AllocFunc allocFunc,
        Function::ExecuteFunc toStringFunc);
void transform_function_and_reeval(Term* term, Term* new_function);
