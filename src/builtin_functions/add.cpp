// Copyright 2008 Paul Hodge

#include <circa.h>

namespace circa {
namespace add_function {

    void evaluate(Term* caller)
    {
        as_float(caller) = as_float(caller->input(0)) + as_float(caller->input(1));
    }

    float get_mutability(Term* term)
    {
        // temp:
        if (!term->hasProperty("mutability"))
            return 1.0;

        return as_float(term->property("mutability"));
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
            totalInputMutability += get_mutability(target->input(i));
        }

        if (totalInputMutability == 0.0) {
            error_occured(caller, "no inputs are mutable");
            return;
        }
    
        float total_delta = as_float(desired) - as_float(target);
    
        for (int i=0; i < numInputs; i++) {
            float mutability = get_mutability(target->input(i));

            float inputDelta = total_delta * mutability / totalInputMutability;
    
            Term* input = target->inputs[i];
    
            apply(&myBranch, APPLY_FEEDBACK,
                RefList(input, float_value(&myBranch, as_float(input) + inputDelta)));
        }
    
        evaluate_branch(myBranch);
    }

    void generateTraining(Branch& branch, Term* subject, Term* desired)
    {
        // find the # of trainable inputs
        int numTrainableInputs = 0;

        for (int i=0; i < subject->numInputs(); i++)
            if (is_trainable(subject->input(i)))
                numTrainableInputs++;

        // if there are no trainable inputs, nothing to do
        if (numTrainableInputs == 0)
            return;

        // delta is just desired - current
        Term* delta = apply(&branch, SUB_FUNC, RefList(desired, subject));

        // if there are multiple trainable inputs, divide up delta
        if (numTrainableInputs > 1) {
            delta = apply(&branch, DIV_FUNC, RefList(delta, float_value(&branch, numTrainableInputs)));
        }

        // pass delta to each trainable input
        for (int i=0; i < subject->numInputs(); i++) {
            Term* input = subject->input(i);
            if (is_trainable(input)) {
                Term* inputDesired = apply(&branch, ADD_FUNC, RefList(input, delta));
                generate_training(branch, input, inputDesired);
            }
        }
    }

    void setup(Branch& kernel)
    {
        ADD_FUNC = import_function(kernel, evaluate, "add(float,float) -> float");
        as_function(ADD_FUNC).pureFunction = true;
        as_function(ADD_FUNC).generateTraining = generateTraining;

        Term* fp_func = import_function(kernel, feedback_propogate,
                "add_feedback_propogate(any,any)");
        as_function(fp_func).stateType = BRANCH_TYPE;
        as_function(ADD_FUNC).feedbackPropogateFunction = fp_func;
    }
}
}
