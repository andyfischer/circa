// Copyright 2008 Paul Hodge

#include <circa.h>

namespace circa {
namespace add_f_function {

    void evaluate(Term* caller)
    {
        float result = 0.0;
        for (int i=0; i < caller->numInputs(); i++)
            result += to_float(caller->input(i));
        as_float(caller) = result;
    }

    void feedback_evaluate(Term* caller)
    {
        Term* target = caller->input(0);
        float desired = to_float(caller->input(1));

        float delta = desired - to_float(target);

        Branch& outputList = as_branch(caller);
        for (int i=0; i < outputList.length(); i++) {
            Term* output = outputList[i];
            Term* outputTarget = target->input(i);
            float balanced_delta = delta * get_feedback_weight(output);
            as_float(output) = to_float(outputTarget) + balanced_delta;
        }
    }

    void setup(Branch& kernel)
    {
        Term* func = import_function(kernel, evaluate, "add_f(float...) : float");
        as_function(func).pureFunction = true;
        as_function(func).feedbackFunc = 
            import_function(kernel, feedback_evaluate, "add_feedback(any, float) : Branch");
    }
}
}