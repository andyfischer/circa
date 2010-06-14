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

    void evaluate(EvalContext* cxt, Term* caller)
    {
        Term* func = as_ref(function_t::get_parameters(caller->function));

#ifdef NEWLIST
        Term* left = caller->input(0);
        Term* right = caller->input(1);
        List* output = (List*) caller;
        int numInputs = left->numElements();

        if (numInputs != right->numElements()) {
            std::stringstream msg;
            msg << "Input lists have different lengths (left has " << numInputs;
            msg << ", right has " << right->numElements() << ")";
            error_occurred(cxt, caller, msg.str());
            return;
        }

        Branch evaluationBranch;
        Term* input0 = apply(evaluationBranch, INPUT_PLACEHOLDER_FUNC, RefList());
        Term* input1 = apply(evaluationBranch, INPUT_PLACEHOLDER_FUNC, RefList());

        Term* evalResult = apply(evaluationBranch, func, RefList(input0, input1));

        output->resize(numInputs);
        mutate(output);

        for (int i=0; i < numInputs; i++) {
            copy(left->getIndex(i), input0);
            copy(right->getIndex(i), input1);
            evaluate_branch(evaluationBranch);

            copy(evalResult, output->get(i));
        }
#else
        Branch& left = as_branch(caller->input(0));
        Branch& right = as_branch(caller->input(1));

        if (left.length() != right.length()) {
            std::stringstream msg;
            msg << "Input lists have different lengths (left has " << left.length();
            msg << ", right has " << right.length() << ")";
            error_occurred(cxt, caller, msg.str());
            return;
        }

        Branch& output = as_branch(caller);

        // Check if our output value has been precreated but not initialized by us.
        if (output.length() > 0 && output[0]->function == VALUE_FUNC)
            output.clear();

        if (output.length() == 0) {
            output.clear();
            for (int i=0; i < left.length(); i++)
                apply(output, func, RefList(left[i], right[i]));
        }

        evaluate_branch(cxt, output);
#endif
    }

    void setup(Branch& kernel)
    {
        Term* func = import_function(kernel, evaluate,
                "vectorize_vv(List,List) -> List");
        function_t::get_specialize_type(func) = specializeType;
    }
}
} // namespace circa
