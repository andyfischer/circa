// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace sin_function {

    void evaluate(Term* caller)
    {
        float input = to_float(caller->input(0));
        as_float(caller) = sin(input);
    }

    void feedback_evaluate(Term* caller)
    {
        // Term* target = caller->input(0);
        float desired = caller->input(1)->toFloat();

        // restrict input to -1..1
        if (desired > 1)
            desired = std::fmod(desired + 1, 2.0f) - 1;

        // TODO: find a value that is in the same period as the target's input

        as_float(caller) = std::asin(desired);
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate, "sin(float) : float");
        as_function(main_func).pureFunction = true;
        as_function(main_func).feedbackFunc = 
            import_function(kernel, feedback_evaluate, "sin_feedback(any, float) : float");
    }
}
}
