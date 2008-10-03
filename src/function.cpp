// Copyright 2008 Andrew Fischer

#include "builtins.h"
#include "bootstrapping.h"
#include "errors.h"
#include "function.h"
#include "list.h"
#include "operations.h"
#include "values.h"

namespace circa {

Function::Function()
  : outputType(NULL),
    stateType(NULL),
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

Function& as_function(Term* term)
{
    if (!is_function(term))
        throw errors::TypeError(term, FUNCTION_TYPE);

    return *((Function*) term->value);
}

void Function::alloc(Term* caller)
{
    caller->value = new Function();
}

void Function::dealloc(Term* caller)
{
    delete (Function*) caller->value;
}

void Function::duplicate(Term* source, Term* dest)
{
    Function::alloc(dest); // necessary?
    as_function(dest) = as_function(source);
}

std::string get_placeholder_name_for_index(int index)
{
    std::stringstream sstream;
    sstream << INPUT_PLACEHOLDER_PREFIX << index;
    return sstream.str();
}

void Function::subroutine_create(Term* caller)
{
    // 0: name (string)
    // 1: inputTypes (list of type)
    // 2: outputType (type)

    /* FIXME
     
    as_string(caller->inputs[0]);
    as_list(caller->inputs[1]);
    as_type(caller->inputs[2]);

    Function& sub = as_function(caller);
    sub.name = as_string(caller->inputs[0]);
    sub.initialize = Function::call_subroutine__initialize;
    sub.evaluate = Function::call_subroutine;
    sub.inputTypes = as_list(caller->inputs[1]).toReferenceList();
    sub.outputType = caller->inputs[2];
    sub.stateType = BRANCH_TYPE;

    // Create input placeholders
    for (unsigned int index=0; index < sub.inputTypes.count(); index++) {
        std::string name = get_placeholder_name_for_index(index);
        Term* placeholder = create_constant(&sub.subroutineBranch,
                sub.inputTypes[index]);
        sub.subroutineBranch.bindName(placeholder, name);
    }
    */
}

void
Function::call_subroutine__initialize(Term* caller)
{
    // todo
}

void
Function::call_subroutine(Term* caller)
{
    // todo
}

void Function::name_input(Term* caller)
{
    recycle_value(caller->inputs[0], caller);
    int index = as_int(caller->inputs[1]);
    std::string name = as_string(caller->inputs[2]);
    Function& sub = as_function(caller);
    Term* inputPlaceholder =
        sub.subroutineBranch.getNamed(get_placeholder_name_for_index(index));
    sub.subroutineBranch.bindName(inputPlaceholder, name);
}

void initialize_functions(Branch* kernel)
{
    quick_create_function(kernel, "subroutine-create",
        Function::subroutine_create,
        ReferenceList(STRING_TYPE,LIST_TYPE,TYPE_TYPE),
        FUNCTION_TYPE);

    quick_create_function(kernel,
        "function-name-input", Function::name_input,
        ReferenceList(FUNCTION_TYPE, INT_TYPE, STRING_TYPE), FUNCTION_TYPE);
}

} // namespace circa
