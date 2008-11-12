
// Copyright 2008 Andrew Fischer

// This file was generated using ../src/builtin_functions/add.source.py.
// You should probably not modify this file directly.

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
    void setup(Branch& kernel)
    {
        Term* main_func = import_c_function(kernel, evaluate,
                "function add(float,float) -> float");
        as_function(main_func).pureFunction = true;

        Term* fp_func = import_c_function(kernel, feedback_propogate,
                "function add-feedback-propogate(any,any)");
        as_function(fp_func).stateType = BRANCH_TYPE;
        as_function(main_func).feedbackPropogateFunction = fp_func;
    }
}
