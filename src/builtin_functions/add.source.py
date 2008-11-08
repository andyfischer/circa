# Copyright 2008 Andrew Fischer

header = "function add(float,float) -> float"
pure = True

evaluate = """
        as_float(caller) = as_float(caller->inputs[0]) + as_float(caller->inputs[1])
"""

feedback_propogate = """
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
"""
