// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {

Term* get_global(std::string name)
{
    if (KERNEL->contains(name))
        return KERNEL->get(name);

    return NULL;
}

Function& get_function_data(Term* function)
{
    if (is_subroutine(function))
        return get_subroutines_function_def(function);
    else
        return as_function(function);
}

void evaluate_term(Term* term)
{
    if (term == NULL)
        throw std::runtime_error("term is NULL");

    term->hasError = false;

    std::string errorMessage;

    if (has_static_error(term)) {
        error_occurred(term, get_static_error_message(term));
        return;
    }

    Function& func = get_function_data(term->function);

    if (func.evaluate == NULL)
        return;
    
    // Make sure we have an allocated value. Allocate one if necessary
    if (!is_value_alloced(term))
        alloc_value(term);

    // Execute the function
    try {
        func.evaluate(term);
    }
    catch (std::exception const& err)
    {
        error_occurred(term, err.what());
    }
}

void evaluate_branch(Branch& branch, Term* errorListener)
{
    for (int index=0; index < branch.length(); index++) {
		Term* term = branch.get(index);
        evaluate_term(term);

        if (term->hasError) {
            std::stringstream out;
            out << "On term " << term_to_raw_string(term) << "\n" << term->getErrorMessage();
            error_occurred(errorListener, out.str());
            return;
        }
    }
}

void error_occurred(Term* errorTerm, std::string const& message)
{
    if (errorTerm == NULL) {
        throw std::runtime_error(message);
        return;
    }

    errorTerm->hasError = true;
    errorTerm->attachErrorMessage(message);
}

Term* apply_and_eval(Branch* branch, Term* function, RefList const& inputs)
{
    Term* result = apply(branch, function, inputs);
    evaluate_term(result);
    return result;
}

Term* apply_and_eval(Branch* branch, std::string const& functionName,
        RefList const &inputs)
{
    Term* function = find_named(branch, functionName);
    if (function == NULL)
        throw std::runtime_error("function not found: "+functionName);

    return apply_and_eval(branch, function, inputs);
}

void resize_list(Branch& list, int numElements, Term* type)
{
    // Add terms if necessary
    for (int i=list.length(); i < numElements; i++)
        create_value(&list, type);

    // Remove terms if necessary
    for (int i=numElements; i < list.length(); i++) {
        list[i] = NULL;
    }

    list.removeNulls();
}

int& as_int(Term* term)
{
    assert_type(term, INT_TYPE);
    alloc_value(term);
    return *((int*) term->value);
}

float& as_float(Term* term)
{
    assert_type(term, FLOAT_TYPE);
    alloc_value(term);
    return *((float*) term->value);
}

float to_float(Term* term)
{
    alloc_value(term);
    if (term->type == FLOAT_TYPE)
        return as_float(term);
    else if (term->type == INT_TYPE)
        return (float) as_int(term);
    else
        throw std::runtime_error("Type mismatch in to_float");
}

bool& as_bool(Term* term)
{
    assert_type(term, BOOL_TYPE);
    alloc_value(term);
    return *((bool*) term->value);
}

std::string& as_string(Term* term)
{
    assert_type(term, STRING_TYPE);
    alloc_value(term);
    return *((std::string*) term->value);
}

bool is_int(Term* term)
{
    return term->type == INT_TYPE;
}

bool is_float(Term* term)
{
    return term->type == FLOAT_TYPE;
}

bool is_bool(Term* term)
{
    return term->type == BOOL_TYPE;
}

bool is_string(Term* term)
{
    return term->type == STRING_TYPE;
}

bool is_ref(Term* term)
{
    return term->type == REF_TYPE;
}

Ref& deref(Term* term)
{
    assert_type(term, REF_TYPE);
    return *((Ref*) term->value);
}

void*& as_void_ptr(Term* term)
{
    assert_type(term, VOID_PTR_TYPE);
    return term->value;
}

} // namespace circa
