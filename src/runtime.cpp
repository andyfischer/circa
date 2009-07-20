// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {

Term* get_global(std::string name)
{
    if (KERNEL->contains(name))
        return KERNEL->get(name);

    return NULL;
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

    EvaluateFunc evaluate = function_t::get_evaluate(term->function);

    if (evaluate == NULL)
        return;
    
    // Make sure we have an allocated value. Allocate one if necessary
    if (!is_value_alloced(term))
        alloc_value(term);

    // Execute the function
    try {
        evaluate(term);
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

Term* apply_and_eval(Branch& branch, Term* function, RefList const& inputs)
{
    Term* result = apply(branch, function, inputs);
    evaluate_term(result);
    return result;
}

Term* apply_and_eval(Branch& branch, std::string const& functionName,
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
        create_value(list, type);

    // Remove terms if necessary
    for (int i=numElements; i < list.length(); i++) {
        list[i] = NULL;
    }

    list.removeNulls();
}

} // namespace circa
