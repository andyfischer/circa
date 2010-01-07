// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace trig_function {

    void evaluate_sin(Term* caller)
    {
        float input = float_input(caller,0);

        // Convert input from 0..1 to 0..2pi
        as_float(caller) = sin(float(input * 2 * M_PI));
    }
    void evaluate_cos(Term* caller)
    {
        float input = float_input(caller,0);

        // Convert input from 0..1 to 0..2pi
        as_float(caller) = cos(float(input * 2 * M_PI));
    }
    void evaluate_tan(Term* caller)
    {
        float input = float_input(caller,0);

        // Convert input from 0..1 to 0..2pi
        as_float(caller) = tan(float(input * 2 * M_PI));
    }
    void evaluate_arcsin(Term* caller)
    {
        float input = float_input(caller,0);
        float result = asin(input);
        as_float(caller) = result / float(2 * M_PI);
    }
    void evaluate_arccos(Term* caller)
    {
        float input = float_input(caller,0);
        float result = acos(input);
        as_float(caller) = result / float(2 * M_PI);
    }
    void evaluate_arctan(Term* caller)
    {
        float input = float_input(caller,0);
        float result = atan(input);
        as_float(caller) = result / float(2 * M_PI);
    }

    void feedback_evaluate_sin(Term* caller)
    {
        // Term* target = caller->input(0);
        float desired = float_input(caller, 1);

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
        Term* sin_func = import_function(kernel, evaluate_sin, "sin(number angle) -> number;"
            "'Trigonometric sin() function. Note that angles are specified in a range of 0..1 (instead of 0..2pi)' end");
        function_t::get_feedback_func(sin_func) = 
            import_function(kernel, feedback_evaluate_sin, "sin_feedback(any, number) -> number");

        Term* cos_func = import_function(kernel, evaluate_cos, "cos(number angle) -> number;"
            "'Trigonometric cos() function. Note that angles are specified in a range of 0..1 (instead of 0..2pi)' end");
        function_t::get_feedback_func(cos_func) = 
            import_function(kernel, feedback_evaluate_cos, "cos_feedback(any, number) -> number");

        import_function(kernel, evaluate_tan, "tan(number angle) -> number;"
            "'Trigonometric tan() function. Note that angles are specified in a range of 0..1 (instead of 0..2pi)' end");
        import_function(kernel, evaluate_arcsin, "arcsin(number) -> number;"
            "'Trigonometric arcsin() function. Note that angles are specified in a range of 0..1 (instead of 0..2pi)' end");
        import_function(kernel, evaluate_arccos, "arccos(number) -> number;"
            "'Trigonometric arccos() function. Note that angles are specified in a range of 0..1 (instead of 0..2pi)' end");
        import_function(kernel, evaluate_arctan, "arctan(number) -> number;"
            "'Trigonometric arctan() function. Note that angles are specified in a range of 0..1 (instead of 0..2pi)' end");
    }
} // namespace trig_function
} // namespace circa
