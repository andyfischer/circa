// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "branch.h"
#include "builtins.h"
#include "errors.h"
#include "function.h"
#include "list.h"
#include "operations.h"
#include "parser.h"
#include "ref_list.h"
#include "ref_map.h"
#include "term.h"
#include "type.h"
#include "values.h"

namespace circa {

void initialize_term(Term* term, Term* function, ReferenceList inputs);

Term* create_term(Branch* branch, Term* function, ReferenceList inputs)
{
    //if (branch == NULL)
    //    throw std::runtime_error("in create_term, branch is NULL");
    if (!is_function(function))
        throw std::runtime_error("in create_term, 2nd arg to create_term must be a function");

    Term* term = new Term();

    if (branch != NULL)
        branch->append(term);

    initialize_term(term, function, inputs);
    
    return term;
}

void initialize_term(Term* term, Term* function, ReferenceList inputs)
{
    if (term == NULL)
        throw std::runtime_error("Term* is NULL");

    if (function == NULL)
        throw std::runtime_error("Function is NULL");

    term->function = function;
    Function& functionData = as_function(function);

    Term* outputType = functionData.outputType;
    Term* stateType = functionData.stateType;

    if (outputType == NULL)
        throw std::runtime_error("outputType is NULL");
        
    if (!is_type(outputType))
        throw std::runtime_error(outputType->findName() + " is not a type");

    if (stateType != NULL && !is_type(stateType))
        throw std::runtime_error(outputType->findName() + " is not a type");

    change_type(term, outputType);

    // Create state (if a state type is defined)
    if (stateType != NULL) {
        term->state = create_var(NULL, stateType);
    }
    else
        term->state = NULL;

    set_inputs(term, inputs);

    // Run the function's initialize (if it has one)
    if (functionData.initialize != NULL) {
        functionData.initialize(term);
    }
}

void set_inputs(Term* term, ReferenceList inputs)
{
    term->inputs = inputs;
}

Term* create_var(Branch* branch, Term* type)
{
    Term *term = create_term(branch, get_var_function(*branch, type), ReferenceList());
    term->stealingOk = false;
    return term;
}

void set_input(Term* term, int index, Term* input)
{
    term->inputs.setAt(index, input);
}

Term* apply_function(Branch& branch, Term* function, ReferenceList inputs)
{
    if (function->needsUpdate)
        function->eval();

    // Check if 'function' is actually a type
    if (is_type(function))
    {
        if (inputs.count() != 0)
            throw std::runtime_error("Multiple inputs in constructor not supported");

        return create_term(&branch, get_var_function(branch, function), inputs);
    }

    // If 'function' is not really a function, see if we can treat it like a function
    if (!is_function(function)) {

        Type* type = as_type(function->type);

        if (!type->memberFunctions.contains(""))
            throw std::runtime_error(std::string("Term ") + function->toString()
                    + " is not a type, and has no default function");

        ReferenceList memberFunctionInputs;
        memberFunctionInputs.append(function);
        memberFunctionInputs.appendAll(inputs);

        return create_term(&branch, type->memberFunctions[""], memberFunctionInputs);
    }

    // Create a term in the normal way
    return create_term(&branch, function, inputs);
}

Term* eval_function(Branch& branch, Term* function, ReferenceList inputs)
{
    Term* result = apply_function(branch, function, inputs);
    result->eval();
    return result;
}

Term* get_var_function(Branch& branch, Term* type)
{
    Term* result = apply_function(branch, VAR_FUNCTION_GENERATOR, ReferenceList(type));
    result->eval();
    return result;
}

bool is_var(Term* term)
{
    return term->function->function == VAR_FUNCTION_GENERATOR;
}

void change_function(Term* term, Term* new_function)
{
    assert_good(term);

    if (new_function->type != FUNCTION_TYPE)
        throw errors::TypeError(new_function, FUNCTION_TYPE);

    term->function = new_function;
}

void remap_pointers(Term* term, ReferenceMap const& map)
{
    assert_good(term);

    term->inputs.remapPointers(map);
    term->function = map.getRemapped(term->function);

    if (as_type(term->type)->remapPointers != NULL)
        as_type(term->type)->remapPointers(term, map);
}

void remap_pointers(Term* term, Term* original, Term* replacement)
{
    ReferenceMap map;
    map[original] = replacement;
    remap_pointers(term, map);
}

void error_occured(Term* errorTerm, std::string const& message)
{
    //std::cout << "error occured: " << message << std::endl;
    errorTerm->pushError(message);
}

} // namespace circa
