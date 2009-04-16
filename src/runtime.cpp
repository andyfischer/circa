// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {

Term* get_global(std::string name)
{
    if (KERNEL->contains(name))
        return KERNEL->getNamed(name);

    return NULL;
}

bool check_valid_type(Function &func, int index, Term* term)
{
    Term* expectedType = func.inputTypes[index];

    if (expectedType == ANY_TYPE)
        return true;

    return type_matches(term, expectedType);
}

void evaluate_term(Term* term)
{
    if (term == NULL)
        throw std::runtime_error("term is NULL");

    term->clearError();

    // Check function
    if (term->function == NULL) {
        error_occured(term, "Function is NULL");
        return;
    }

    if (!is_function(term->function)) {
        error_occured(term, "term->function is not a function");
        return;
    }

    Function& func = as_function(term->function);

    if (func.evaluate == NULL)
        return;

    // Check each input. Make sure:
    //  1) it is not null
    //  2) it is up-to-date
    //  3) it has a non-null value
    //  4) it has no errors
    //  5) it has the correct type
    for (unsigned int inputIndex=0; inputIndex < term->inputs.count(); inputIndex++)
    {
        int effectiveIndex = inputIndex;
        if (func.variableArgs)
            effectiveIndex = 0;

        Term* input = term->inputs[inputIndex];
        Function::InputProperties inputProps = func.getInputProperties(effectiveIndex);
         
        if (input == NULL && !inputProps.meta) {
            std::stringstream message;
            message << "Input " << inputIndex << " is NULL";
            error_occured(term, message.str());
            return;
        }

        if (!is_value_alloced(input) && !inputProps.meta) {
            std::stringstream message;
            message << "Input " << inputIndex << " has NULL value";
            error_occured(term, message.str());
            return;
        }

        if (input->hasError() && !inputProps.meta) {
            std::stringstream message;
            message << "Input " << inputIndex << " has an error";
            error_occured(term, message.str());
            return;
        }
        
        // Check type
        if (!check_valid_type(func, effectiveIndex, input)) {
            std::stringstream message;
            message << "Runtime type error: input " << inputIndex << " has type "
                << as_type(input->type).name;
            error_occured(term, message.str());
            return;
        }

        // Possibly evaluate this input if needed
        if (!inputProps.meta && input->needsUpdate) {
            assert(term != input); // prevent infinite recursion
            evaluate_term(input);
        }
    }
    
    // Make sure we have an allocated value. Allocate one if necessary
    if (!is_value_alloced(term))
        alloc_value(term);

    // Execute the function
    try {
        func.evaluate(term);
        term->needsUpdate = false;
    }
    catch (std::exception const& err)
    {
        error_occured(term, err.what());
    }
}

void evaluate_branch(Branch& branch, Term* errorListener)
{
    int count = branch.numTerms();
    for (int index=0; index < count; index++) {
		Term* term = branch.get(index);
        evaluate_term(term);

        if (term->hasError()) {
            std::stringstream out;
            out << "On term " << term_to_raw_string(term) << "\n" << term->getErrorMessage();
            error_occured(errorListener, out.str());
            return;
        }
    }
}

void error_occured(Term* errorTerm, std::string const& message)
{
    if (errorTerm == NULL)
        throw std::runtime_error(message);

    errorTerm->pushError(message);
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
    Term* function = find_named(branch,functionName);
    if (function == NULL)
        throw std::runtime_error("function not found: "+functionName);

    return apply_and_eval(branch, function, inputs);
}

int& as_int(Term* term)
{
    assert_type(term, INT_TYPE);
    assert(term->value != NULL);
    return *((int*) term->value);
}

float& as_float(Term* term)
{
    assert_type(term, FLOAT_TYPE);
    assert(term->value != NULL);
    return *((float*) term->value);
}

float to_float(Term* term)
{
    if (term->type == FLOAT_TYPE)
        return as_float(term);
    else if (term->type == INT_TYPE)
        return as_int(term);
    else
        throw std::runtime_error("Type mismatch in to_float");
}

bool& as_bool(Term* term)
{
    assert_type(term, BOOL_TYPE);
    assert(term->value != NULL);
    return *((bool*) term->value);
}

std::string& as_string(Term* term)
{
    assert_type(term, STRING_TYPE);
    assert(term->value != NULL);
    return *((std::string*) term->value);
}

// todo: add these for all primitives
bool is_string(Term* term)
{
    return term->type == STRING_TYPE;
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
