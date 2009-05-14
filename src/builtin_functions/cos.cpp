// Copyright 2008 Paul Hodge

#include "circa.h"

#include "math.h"

namespace circa {
namespace cos_function {

    void evaluate(Term* caller)
    {
        float input = to_float(caller->input(0));
        as_float(caller) = cos(input);
    }

    void feedback_evaluate(Term* caller)
    {
        // Term* target = caller->input(0);
        float desired = caller->input(1)->toFloat();

        // restrict input to -1..1
        if (desired > 1)
            desired = std::fmod(desired + 1, 2.0f) - 1;

        // TODO: find a value that is in the same period as the target's input

        as_float(caller) = std::acos(desired);
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate, "cos(float) : float");
        as_function(main_func).pureFunction = true;
        as_function(main_func).feedbackFunc = 
            import_function(kernel, feedback_evaluate, "cos_feedback(any, float) : float");
    }
}
}
