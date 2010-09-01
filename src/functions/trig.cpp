// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace trig_function {

    CA_FUNCTION(evaluate_sin)
    {
        float input = FLOAT_INPUT(0);

        // Convert input from 0..1 to 0..2pi
        make_float(OUTPUT, sin(float(input * 2 * M_PI)));
    }
    CA_FUNCTION(evaluate_cos)
    {
        float input = FLOAT_INPUT(0);

        // Convert input from 0..1 to 0..2pi
        make_float(OUTPUT, cos(float(input * 2 * M_PI)));
    }
    CA_FUNCTION(evaluate_tan)
    {
        float input = FLOAT_INPUT(0);

        // Convert input from 0..1 to 0..2pi
        make_float(OUTPUT, tan(float(input * 2 * M_PI)));
    }
    CA_FUNCTION(evaluate_arcsin)
    {
        float input = FLOAT_INPUT(0);
        float result = asin(input);
        make_float(OUTPUT, result / float(2 * M_PI));
    }
    CA_FUNCTION(evaluate_arccos)
    {
        float input = FLOAT_INPUT(0);
        float result = acos(input);
        make_float(OUTPUT, result / float(2 * M_PI));
    }
    CA_FUNCTION(evaluate_arctan)
    {
        float input = FLOAT_INPUT(0);
        float result = atan(input);
        make_float(OUTPUT, result / float(2 * M_PI));
    }

    CA_FUNCTION(feedback_evaluate_sin)
    {
        // Term* target = INPUT(0);
        float desired = FLOAT_INPUT(1);

        // restrict input to -1..1
        if (desired > 1)
            desired = std::fmod(desired + 1, 2.0f) - 1;

        // TODO: find a value that is in the same period as the target's input

        float result = std::asin(desired);

        // Map result from radians into range of 0..1
        make_float(OUTPUT, result / float(2 * M_PI));
    }

    CA_FUNCTION(feedback_evaluate_cos)
    {
        // Term* target = INPUT(0);
        float desired = FLOAT_INPUT(1);

        // restrict input to -1..1
        if (desired > 1)
            desired = std::fmod(desired + 1, 2.0f) - 1;

        // TODO: find a value that is in the same period as the target's input

        float result = std::acos(desired);

        // Map result from radians into range of 0..1
        make_float(OUTPUT, result / float(2 * M_PI));
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
