// Copyright 2008 Paul Hodge

#include "builtins.h"
#include "bootstrapping.h"
#include "errors.h"
#include "function.h"
#include "operations.h"

namespace circa {

Function::Function()
  : outputType(NULL),
    stateType(NULL),
    recycleInput(-1),
    pureFunction(false),
    variableArgs(false),
    initialize(NULL),
    evaluate(NULL),
    feedbackAccumulationFunction(NULL),
    feedbackPropagationFunction(NULL)
{
}

bool is_function(Term* term)
{
    return is_instance(term, FUNCTION_TYPE);
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

void Function_duplicate(Term* source, Term* dest)
{
    Function_alloc(dest);
    *as_function(dest) = *as_function(source);
}

void Function_dealloc(Term* caller)
{
    delete as_function(caller);
}

void function_recycle_input(Term* caller)
{
    recycle_value(caller->inputs[0], caller);
    as_function(caller)->recycleInput = as_int(caller->inputs[1]);
}

void initialize_functions(Branch* kernel)
{
    Term* set_recycle_input = quick_create_function(kernel, "function-recycle-input",
            function_recycle_input, TermList(FUNCTION_TYPE,INT_TYPE),
            FUNCTION_TYPE);
    as_function(set_recycle_input)->recycleInput = 0;
}

} // namespace circa
