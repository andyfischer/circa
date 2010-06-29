// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {

inline void evaluate_term(EvalContext* cxt, Term* term)
{
    assert(cxt != NULL);
    assert(term != NULL);

    EvaluateFunc evaluate = function_t::get_evaluate(term->function);

    if (evaluate == NULL)
        return;

    // Execute the function
    try {
#ifdef NEW_EVALUATE
        evaluate(cxt, term, term->function, term->inputs, term);
#else
        evaluate(cxt, term);
#endif
    }
    catch (std::exception const& err)
    {
        error_occurred(cxt, term, err.what());
    }
}

void evaluate_term(Term* term)
{
    EvalContext context;
    evaluate_term(&context, term);
}

void evaluate_branch(EvalContext* context, Branch& branch)
{
    assert(context != NULL);

    for (int index=0; index < branch.length(); index++) {
		Term* term = branch.get(index);
        evaluate_term(context, term);

        if (context->errorOccurred)
            break;
    }
}

EvalContext evaluate_branch(Branch& branch)
{
    EvalContext context;
    evaluate_branch(&context, branch);
    return context;
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
    // TODO
    return true;
}

} // namespace circa
