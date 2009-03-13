// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace mult_function {

    void evaluate(Term* caller)
    {
        as_float(caller) = as_float(caller->input(0)) * as_float(caller->input(1));
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

        Term* delta = apply_function(&branch, SUB_FUNC, RefList(desired, subject));

        // for each input, send a delta divided by the product of all other inputs
        for (int i=0; i < subject->numInputs(); i++) {
            Term* input = subject->input(i);
            if (!is_trainable(input))
                continue;

            // this "product of all other inputs" assumes that we only have 2 inputs
            Term* divisor = i == 0 ? subject->input(1) : subject->input(0);

            Term* inputDelta = apply_function(&branch, DIV_FUNC, RefList(delta, divisor));
            Term* inputDesired = apply_function(&branch, ADD_FUNC, RefList(input, inputDelta));

            generate_training(branch, input, inputDesired);
        }
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate,
                "function mult(float,float) -> float");
        as_function(main_func).pureFunction = true;
        as_function(main_func).generateTraining = generateTraining;
    }
}
} // namespace circa
