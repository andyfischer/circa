// Copyright 2008 Paul Hodge

#include <circa.h>

namespace circa {
namespace add_function {

    void evaluate(Term* caller)
    {
        float result = 0.0;
        for (int i=0; i < caller->numInputs(); i++)
            result += to_float(caller->input(i));
        as_float(caller) = result;
    }

    void generateFeedback(Branch& branch, Term* subject, Term* desired)
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
                generate_feedback(branch, input, inputDesired);
            }
        }
    }

    void generateFeedbackNew(Branch& branch, FeedbackOperation& operation, Term* subject)
    {
        // find the # of trainable inputs
        int numTrainableInputs = 0;

        for (int i=0; i < subject->numInputs(); i++)
            if (is_trainable(subject->input(i)))
                numTrainableInputs++;

        // if there are no trainable inputs, nothing to do
        if (numTrainableInputs == 0)
            return;

        // Compute delta as desired - current
        Ref desired = operation.getFeedback(subject, DESIRED_VALUE_FEEDBACK);
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
                operation.sendFeedback(input, inputDesired, DESIRED_VALUE_FEEDBACK);
            }
        }
    }

    void setup(Branch& kernel)
    {
        ADD_FUNC = import_function(kernel, evaluate, "add(float...) : float");
        as_function(ADD_FUNC).pureFunction = true;
        as_function(ADD_FUNC).generateFeedback = generateFeedback;
        as_function(ADD_FUNC).generateFeedbackNew = generateFeedbackNew;
    }
}
}
