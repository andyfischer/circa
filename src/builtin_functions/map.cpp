// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace map_function {

    void evaluate(EvalContext* cxt, Term* caller)
    {
        Term* func = caller->input(0);
        List* inputs = (List*) caller->input(1);

        if (is_function_stateful(func)) {
            error_occurred(cxt, caller, "map() not yet supported on a stateful function");
            return;
        }

        int numInputs = inputs->numElements();

        Branch evaluationBranch;
        Term* evalInput = apply(evaluationBranch, INPUT_PLACEHOLDER_FUNC, RefList());
        Term* evalResult = apply(evaluationBranch, func, RefList(evalInput));

#ifdef NEWLIST
        List* list = (List*) caller;
        list->resize(numInputs);

        for (int i=0; i < numInputs; i++) {
            copy(inputs->getIndex(i), evalInput);
            evaluate_branch(evaluationBranch);

            copy(evalResult, list->get(i));
        }
#else
        Branch& output = as_branch(caller);
        output.clear();

        for (int i=0; i < numInputs; i++) {
            copy(inputs->getIndex(i), evalInput);
            evaluate_branch(evaluationBranch);

            Term* outputElement = create_value(output, evalResult->type);
            copy(evalResult, outputElement);
        }
#endif


#if 0
        Old branch-based version:

        // Create term if necessary
        for (int i=output.length(); i < inputs.length(); i++)
            apply(output, func, RefList(inputs[i]));

        // Remove extra terms if necessary
        for (int i=inputs.length(); i < output.length(); i++)
            output.set(i, NULL);

        output.removeNulls();

        evaluate_branch(cxt, output);
#endif
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "def map(any,Indexable) -> List");
    }
}
} // namespace circa
