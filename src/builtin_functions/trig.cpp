// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "circa.h"

namespace circa {
namespace trig_function {

    void evaluate_sin(Term* caller)
    {
        float input = to_float(caller->input(0));

        // Convert input from 0..1 to 0..2pi
        as_float(caller) = sin(float(input * 2 * M_PI));
    }

    void evaluate_cos(Term* caller)
    {
        float input = to_float(caller->input(0));

        // Convert input from 0..1 to 0..2pi
        as_float(caller) = cos(float(input * 2 * M_PI));
    }

    void feedback_evaluate_sin(Term* caller)
    {
        // Term* target = caller->input(0);
        float desired = caller->input(1)->toFloat();

        // restrict input to -1..1
        if (desired > 1)
            desired = std::fmod(desired + 1, 2.0f) - 1;

        // TODO: find a value that is in the same period as the target's input

        float result = std::asin(desired);

        // Map result from radians into range of 0..1
        as_float(caller) = result / float(2 * M_PI);
    }

    void feedback_evaluate_cos(Term* caller)
    {
        // Term* target = caller->input(0);
        float desired = caller->input(1)->toFloat();

        // restrict input to -1..1
        if (desired > 1)
            desired = std::fmod(desired + 1, 2.0f) - 1;

        // TODO: find a value that is in the same period as the target's input

        float result = std::acos(desired);

        // Map result from radians into range of 0..1
        as_float(caller) = result / float(2 * M_PI);
    }

    void setup(Branch& kernel)
    {
        Term* sin_func = import_function(kernel, evaluate_sin, "sin(number) : number");
        function_t::get_feedback_func(sin_func) = 
            import_function(kernel, feedback_evaluate_sin, "sin_feedback(any, number) : number");

        Term* cos_func = import_function(kernel, evaluate_cos, "cos(number) : number");
        function_t::get_feedback_func(cos_func) = 
            import_function(kernel, feedback_evaluate_cos, "cos_feedback(any, number) : number");
    }
}
}
