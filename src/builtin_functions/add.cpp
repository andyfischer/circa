// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace add_function {

    void evaluate(Term* caller)
    {
        as_float(caller) = as_float(caller->input(0)) + as_float(caller->input(1));
    }

    void feedback_propogate(Term* caller)
    {
        Term* target = caller->input(0);
        Term* desired = caller->input(1);
        Branch& myBranch = as_branch(caller->state);
        myBranch.clear();

        int numInputs = target->inputs.count();

        if (numInputs == 0)
            return;

        // find the total of input mutability
        float totalInputMutability = 0.0;
        for (int i=0; i < numInputs; i++) {
            totalInputMutability += as_float(target->input(i)->property("mutability"));
        }

        if (totalInputMutability == 0.0) {
            error_occured(caller, "no inputs are mutable");
            return;
        }
    
        float total_delta = as_float(desired) - as_float(target);
    
    
        for (int i=0; i < numInputs; i++) {
            float mutability = as_float(target->input(i)->property("mutability"));

            float inputDelta = total_delta * mutability / totalInputMutability;
    
            Term* input = target->inputs[i];
    
            apply_function(myBranch, APPLY_FEEDBACK,
                ReferenceList(input, float_value(myBranch, as_float(input) + inputDelta)));
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
}
