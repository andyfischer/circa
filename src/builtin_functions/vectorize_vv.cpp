// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace vectorize_vv_function {

    Term* specializeType(Term* caller)
    {
        Term* lhsType = caller->input(0)->type;
        if (list_t::is_list_based_type(type_contents(lhsType)))
            return lhsType;
        return LIST_TYPE;
    }

    CA_FUNCTION(evaluate)
    {
        Term* func = as_ref(function_t::get_parameters(FUNCTION));

        TaggedValue* left = INPUT(0);
        TaggedValue* right = INPUT(1);
        List* output = (List*) OUTPUT;
        int numInputs = left->numElements();

        if (numInputs != right->numElements()) {
            std::stringstream msg;
            msg << "Input lists have different lengths (left has " << numInputs;
            msg << ", right has " << right->numElements() << ")";
            error_occurred(CONTEXT, CALLER, msg.str());
            return;
        }

        Branch evaluationBranch;
        Term* input0 = apply(evaluationBranch, INPUT_PLACEHOLDER_FUNC, RefList());
        Term* input1 = apply(evaluationBranch, INPUT_PLACEHOLDER_FUNC, RefList());

        Term* evalResult = apply(evaluationBranch, func, RefList(input0, input1));

        output->resize(numInputs);
        touch(output);

        for (int i=0; i < numInputs; i++) {
            copy(left->getIndex(i), input0);
            copy(right->getIndex(i), input1);
            evaluate_branch(evaluationBranch);

            copy(evalResult, output->get(i));
        }
    }

    void setup(Branch& kernel)
    {
        Term* func = import_function(kernel, evaluate,
                "vectorize_vv(List,List) -> List");
        function_t::get_specialize_type(func) = specializeType;
    }
}
} // namespace circa
