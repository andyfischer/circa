
#include "codeunit.h"
#include "function.h"
#include "term.h"
#include "type.h"


Term* create_term(Term* function, TermList inputs);
void initialize_term(Term* term, Term* function, TermList inputs);
void set_inputs(Term* term, TermList inputs);
Term* create_constant(Term* type);
void change_type(Term* term, Term* type);
void specialize_type(Term* term, Term* type);
void set_input(Term* term, int index, Term* input);
void execute(Term* term);

// Examine 'function' and 'inputs' and returns a result term. A few things
// may happen here:
//  1. We might re-use an existing term
//  2. 'function' might be a type (create an empty instance)
//  3. We might specialize an overloaded function
//  4. add more stuff here
Term* apply_function(Term* function, TermList inputs);

// Fetch the const function for this type
Term* get_const_function(Term* type);

// Create a new Type with the given properties. Also binds the name.
Term* quick_create_type(CodeUnit* code, string name, Type::AllocFunc allocFunc,
        Function::ExecuteFunc toStringFunc, Type::CopyFunc copyFunc = NULL);

// Create a new Function with the given properties. Also binds the name.
Term* quick_create_function(CodeUnit* code, string name, Function::ExecuteFunc executeFunc,
        TermList inputTypes, Term* outputType);

void change_function(Term* term, Term* new_function);
void copy_term(Term* source, Term* dest);

Term* constant_string(std::string s);
Term* constant_int(int i);
