// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace trig_function {

    float radians_to_degrees(float radians) { return radians * 180.0f / M_PI; }
    float degrees_to_radians(float unit) { return unit * M_PI / 180.0f; }

    void evaluate_sin(caStack* stack)
    {
        float input = circa_float_input(stack, 0);

        set_float(circa_output(stack, 0), sin(degrees_to_radians(input)));
    }
    void evaluate_cos(caStack* stack)
    {
        float input = circa_float_input(stack, 0);

        set_float(circa_output(stack, 0), cos(degrees_to_radians(input)));
    }
    void evaluate_tan(caStack* stack)
    {
        float input = circa_float_input(stack, 0);

        set_float(circa_output(stack, 0), tan(degrees_to_radians(input)));
    }
    void evaluate_arcsin(caStack* stack)
    {
        float result = asin(circa_float_input(stack, 0));
        set_float(circa_output(stack, 0), radians_to_degrees(result));
    }
    void evaluate_arccos(caStack* stack)
    {
        float result = acos(circa_float_input(stack, 0));
        set_float(circa_output(stack, 0), radians_to_degrees(result));
    }
    void evaluate_arctan(caStack* stack)
    {
        float result = atan(circa_float_input(stack, 0));
        set_float(circa_output(stack, 0), radians_to_degrees(result));
    }

    void setup(Block* kernel)
    {
        import_function(kernel, evaluate_sin, "sin(number degrees) -> number;"
            "'Trigonometric sin() function");
        import_function(kernel, evaluate_cos, "cos(number degrees) -> number;"
            "'Trigonometric cos() function'");
        import_function(kernel, evaluate_tan, "tan(number degrees) -> number;"
            "'Trigonometric tan() function'");
        import_function(kernel, evaluate_arcsin, "arcsin(number n) -> number;"
            "'Trigonometric arcsin() function'");
        import_function(kernel, evaluate_arccos, "arccos(number n) -> number;"
            "'Trigonometric arccos() function'");
        import_function(kernel, evaluate_arctan, "arctan(number n) -> number;"
            "'Trigonometric arctan() function'");
    }
} // namespace trig_function
} // namespace circa
