// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace mult_function {

    void evaluate(Term* caller)
    {
        as_float(caller) = to_float(caller->input(0)) * to_float(caller->input(1));
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

        Term* delta = apply(&branch, SUB_FUNC, RefList(desired, subject));

        if (numTrainableInputs > 1) {
            delta = apply(&branch, DIV_FUNC, RefList(delta,
                        float_value(&branch, numTrainableInputs)));
        }

        // for each input, send a delta divided by the product of all other inputs
        for (int i=0; i < subject->numInputs(); i++) {
            Term* input = subject->input(i);
            if (!is_trainable(input))
                continue;

            // this "product of all other inputs" assumes that we only have 2 inputs
            Term* divisor = i == 0 ? subject->input(1) : subject->input(0);

            Term* inputDelta = apply(&branch, DIV_FUNC, RefList(delta, divisor));
            Term* inputDesired = apply(&branch, ADD_FUNC, RefList(input, inputDelta));

            generate_training(branch, input, inputDesired);
        }
    }

    void setup(Branch& kernel)
    {
        MULT_FUNC = import_function(kernel, evaluate, "mult(float,float) : float");
        as_function(MULT_FUNC).pureFunction = true;
        as_function(MULT_FUNC).generateTraining = generateTraining;
    }
}
} // namespace circa
