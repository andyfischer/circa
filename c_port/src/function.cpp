
#include "builtins.h"
#include "bootstrapping.h"
#include "errors.h"
#include "function.h"

namespace circa {

Function::Function()
{
    stateType = NULL;
    recycleInput = -1;
    pureFunction = false;
    variableArgs = false;
    initialize = NULL;
    execute = NULL;
}

bool is_function(Term* term)
{
    return (term->type == FUNCTION_TYPE) || (term->type == SUBROUTINE_TYPE);
}

Function* as_function(Term* term)
{
    if (!is_function(term))
        throw errors::TypeError(term, FUNCTION_TYPE);

    return (Function*) term->value;
}

void Function_alloc(Term* caller)
{
    caller->value = new Function();
}

void Function_dealloc(Term* caller)
{
    delete as_function(caller);
}

void function_recycle_input(Term* caller)
{
    // Recycles input 0
    as_function(caller)->recycleInput = as_int(caller->inputs[1]);
}

void initialize_functions(Branch* kernel)
{
    Term* set_recycle_input = quick_create_function(kernel, "function-recycle-input",
            function_recycle_input, TermList(get_global("Function"),get_global("int")),
            get_global("Function"));
    as_function(set_recycle_input)->recycleInput = 0;
}

} // namespace circa
