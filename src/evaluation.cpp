// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {

void evaluate_term(Term* term)
{
    if (term == NULL)
        throw std::runtime_error("term is NULL");

    clear_error(term);

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

        if (term->hasError()) {
            nested_error_occurred(errorListener);
            return;
        }
    }
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

void evaluate_without_side_effects(Term* term)
{
    // TODO: Should actually check if the function has side effects.
    for (int i=0; i < term->numInputs(); i++) {
        Term* input = term->input(i);
        if (input->owningBranch == term->owningBranch)
            evaluate_without_side_effects(input);
    }

    evaluate_term(term);
}

bool has_been_evaluated(Term* term)
{
    // Not the best way of checking:
    return is_value_alloced(term);
}

} // namespace circa
