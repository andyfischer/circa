// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace map_function {

    CA_FUNCTION(evaluate)
    {
        Term* func = INPUT_TERM(0);
        List* inputs = List::checkCast(INPUT(1));

        if (is_function_stateful(func)) {
            error_occurred(CONTEXT, CALLER,
                    "map() not yet supported on a stateful function");
            return;
        }

        int numInputs = inputs->numElements();

        Branch evaluationBranch;
        Term* evalInput = apply(evaluationBranch, INPUT_PLACEHOLDER_FUNC, RefList());
        Term* evalResult = apply(evaluationBranch, func, RefList(evalInput));

        List* list = List::checkCast(OUTPUT);
        list->resize(numInputs);

        for (int i=0; i < numInputs; i++) {
            copy(inputs->getIndex(i), evalInput);
            evaluate_branch(evaluationBranch);

            copy(evalResult, list->get(i));
        }
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "def map(any,Indexable) -> List");
    }
}
} // namespace circa
