// Copyright 2008 Paul Hodge

#include "circa.h"

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
    
            apply_function(&myBranch, APPLY_FEEDBACK,
                RefList(input, float_value(myBranch, as_float(input) + inputDelta)));
        }
    
        evaluate_branch(myBranch);
    }

    void generate_training(Branch& branch, Term* subject, Term* target, RefList& trainableList)
    {
        // find the # of trainable inputs
        int numTrainableInputs = 0;

        for (int i=0; i < subject->numInputs(); i++) {
            if (trainableList.contains(subject->input(i)))
                numTrainableInputs++;
        }

        // if there are no trainable inputs, nothing to do
        // (perhaps we could spit out a warning or something)
        if (numTrainableInputs == 0)
            return;

        // if there is one trainable input, just pass this target directly on
        if (numTrainableInputs == 1) {
            Term* trainableInput = NULL;
            for (int i=0; i < subject->numInputs(); i++) {
                if (trainableList.contains(subject->input(i))) {
                    trainableInput = subject->input(i);
                    break;
                }
            }

            // todo..

        }
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate,
                "function add(float,float) -> float");
        as_function(main_func).pureFunction = true;
        as_function(main_func).generateTraining = generate_training;

        Term* fp_func = import_function(kernel, feedback_propogate,
                "function add-feedback-propogate(any,any)");
        as_function(fp_func).stateType = BRANCH_TYPE;
        as_function(main_func).feedbackPropogateFunction = fp_func;
    }
}
}
