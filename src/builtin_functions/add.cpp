// Copyright 2008 Paul Hodge

namespace add_function {

    void evaluate(Term* caller)
    {
        as_float(caller) = as_float(caller->inputs[0]) + as_float(caller->inputs[1]);
    }
    void feedback_propogate(Term* caller)
    {
        Term* target = caller->inputs[0];
        Term* desired = caller->inputs[1];
        Branch& myBranch = as_branch(caller->state);
        myBranch.clear();

        // for now, send 1/n of the delta to each input
        float delta = as_float(desired) - as_float(target);

        int numInputs = target->inputs.count();

        if (numInputs == 0)
            return;

        for (int i=0; i < numInputs; i++) {
            float inputDelta = delta / numInputs;

            Term* input = target->inputs[i];

            apply_function(myBranch, APPLY_FEEDBACK,
                ReferenceList(input, float_var(myBranch, as_float(input) + inputDelta)));
        }

        evaluate_branch(myBranch);
    }
    static void setup(Branch* kernel)
    {
        ADD_FUNC = quick_create_function(kernel, "add", evaluate,
            ReferenceList(FLOAT_TYPE, FLOAT_TYPE), FLOAT_TYPE);
        as_function(ADD_FUNC).pureFunction = true;

        Term* fp_func =
            quick_create_function(kernel, "add-feedback-propogate",
                feedback_propogate,
                ReferenceList(ANY_TYPE, ANY_TYPE), VOID_TYPE);
        as_function(fp_func).stateType = BRANCH_TYPE;

        as_function(ADD_FUNC).feedbackPropogateFunction = fp_func;
    }
}
