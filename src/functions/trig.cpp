// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"

namespace circa {
namespace trig_function {

    float radians_to_degrees(float radians) { return radians * 180.0 / M_PI; }
    float degrees_to_radians(float unit) { return unit * M_PI / 180.0; }

    CA_FUNCTION(evaluate_sin)
    {
        float input = FLOAT_INPUT(0);

        set_float(OUTPUT, sin(degrees_to_radians(input)));
    }
    CA_FUNCTION(evaluate_cos)
    {
        float input = FLOAT_INPUT(0);

        set_float(OUTPUT, cos(degrees_to_radians(input)));
    }
    CA_FUNCTION(evaluate_tan)
    {
        float input = FLOAT_INPUT(0);

        set_float(OUTPUT, tan(degrees_to_radians(input)));
    }
    CA_FUNCTION(evaluate_arcsin)
    {
        float result = asin(FLOAT_INPUT(0));
        set_float(OUTPUT, radians_to_degrees(result));
    }
    CA_FUNCTION(evaluate_arccos)
    {
        float result = acos(FLOAT_INPUT(0));
        set_float(OUTPUT, radians_to_degrees(result));
    }
    CA_FUNCTION(evaluate_arctan)
    {
        float result = atan(FLOAT_INPUT(0));
        set_float(OUTPUT, radians_to_degrees(result));
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

        set_float(OUTPUT, radians_to_degrees(result));
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

        set_float(OUTPUT, radians_to_degrees(result));
    }

    void setup(Branch* kernel)
    {
        Term* sin_func = import_function(kernel, evaluate_sin, "sin(number degrees) -> number;"
            "'Trigonometric sin() function");
        get_function_attrs(sin_func)->feedbackFunc = 
            import_function(kernel, feedback_evaluate_sin, "sin_feedback(any, number) -> number");

        Term* cos_func = import_function(kernel, evaluate_cos, "cos(number degrees) -> number;"
            "'Trigonometric cos() function'");
        get_function_attrs(cos_func)->feedbackFunc = 
            import_function(kernel, feedback_evaluate_cos, "cos_feedback(any, number) -> number");

        import_function(kernel, evaluate_tan, "tan(number degrees) -> number;"
            "'Trigonometric tan() function'");
        import_function(kernel, evaluate_arcsin, "arcsin(number) -> number;"
            "'Trigonometric arcsin() function'");
        import_function(kernel, evaluate_arccos, "arccos(number) -> number;"
            "'Trigonometric arccos() function'");
        import_function(kernel, evaluate_arctan, "arctan(number) -> number;"
            "'Trigonometric arctan() function'");
    }
} // namespace trig_function
} // namespace circa
