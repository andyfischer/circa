
#include "codeunit.h"
#include "function.h"
#include "term.h"
#include "type.h"


void initialize_term(Term* term, Term* function);
void change_type(Term* term, Term* type);
void specialize_type(Term* term, Term* type);
void set_inputs(Term* term, TermList inputs);
Term* quick_create_type(CodeUnit* code, string name, Type::AllocFunc allocFunc,
        Function::ExecuteFunc toStringFunc);
void transform_function_and_reeval(Term* term, Term* new_function);
