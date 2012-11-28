// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include <algorithm>

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace math_function {

    void max_f(caStack* stack)
    {
        set_float(circa_output(stack, 0),
                std::max(circa_float_input(stack, 0), circa_float_input(stack, 1)));
    }

    void max_i(caStack* stack)
    {
        set_int(circa_output(stack, 0),
                std::max(circa_int_input(stack, 0), circa_int_input(stack, 1)));
    }

    void min_f(caStack* stack)
    {
        set_float(circa_output(stack, 0),
                std::min(circa_float_input(stack, 0), circa_float_input(stack, 1)));
    }

    void min_i(caStack* stack)
    {
        set_int(circa_output(stack, 0),
                std::min(circa_int_input(stack, 0), circa_int_input(stack, 1)));
    }

    void remainder_i(caStack* stack)
    {
        set_int(circa_output(stack, 0), circa_int_input(stack, 0) % circa_int_input(stack, 1));
    }

    void remainder_f(caStack* stack)
    {
        set_float(circa_output(stack, 0), fmodf(circa_float_input(stack, 0), circa_float_input(stack, 1)));
    }

    // We compute mod() using floored division. This is different than C and many
    // C-like languages which use truncated division. See this page for an explanation
    // of the difference:
    // http://en.wikipedia.org/wiki/Modulo_operation
    //
    // For a function that works the same as C's modulo, use remainder() . The % operator
    // also uses remainder(), so that it works the same as C's % operator.

    void mod_i(caStack* stack)
    {
        int a = circa_int_input(stack, 0);
        int n = circa_int_input(stack, 1);

        int out = a % n;
        if (out < 0)
            out += n;

        set_int(circa_output(stack, 0), out);
    }

    void mod_f(caStack* stack)
    {
        float a = circa_float_input(stack, 0);
        float n = circa_float_input(stack, 1);

        float out = fmodf(a, n);

        if (out < 0)
            out += n;

        set_float(circa_output(stack, 0), out);
    }

    void round(caStack* stack)
    {
        float input = circa_float_input(stack, 0);
        if (input > 0.0)
            set_int(circa_output(stack, 0), int(input + 0.5));
        else
            set_int(circa_output(stack, 0), int(input - 0.5));
    }

    void floor(caStack* stack)
    {
        set_int(circa_output(stack, 0), (int) std::floor(circa_float_input(stack, 0)));
    }

    void ceil(caStack* stack)
    {
        set_int(circa_output(stack, 0), (int) std::ceil(circa_float_input(stack, 0)));
    }

    void average(caStack* stack)
    {
        caValue* args = circa_input(stack, 0);
        int count = circa_count(args);
        caValue* out = circa_output(stack, 0);

        if (count == 0) {
            set_float(out, 0);
            return;
        }

        float sum = 0;
        for (int i=0; i < count; i++)
            sum += to_float(circa_index(args, i));

        set_float(out, sum / count);
    }

    void pow(caStack* stack)
    {
        set_int(circa_output(stack, 0),
                (int) std::pow((float) circa_int_input(stack, 0), circa_int_input(stack, 1)));
    }

    void sqr(caStack* stack)
    {
        float in = circa_float_input(stack, 0);
        set_float(circa_output(stack, 0), in * in);
    }
    void cube(caStack* stack)
    {
        float in = circa_float_input(stack, 0);
        set_float(circa_output(stack, 0), in * in * in);
    }

    void sqrt(caStack* stack)
    {
        set_float(circa_output(stack, 0), std::sqrt(circa_float_input(stack, 0)));
    }

    void log(caStack* stack)
    {
        set_float(circa_output(stack, 0), std::log(circa_float_input(stack, 0)));
    }

    void setup(Block* kernel)
    {
        import_function(kernel, max_f, "max_f(number a,number b) -> number; 'Maximum of two numbers'");
        import_function(kernel, max_i, "max_i(int a,int b) -> int; 'Maximum of two integers'");
        import_function(kernel, min_f, "min_f(number a,number b) -> number; 'Minimum of two numbers'");
        import_function(kernel, min_f, "min_i(int a,int b) -> int; 'Minimum of two integers'");
        import_function(kernel, remainder_i, "remainder_i(int a,int b) -> int");
        import_function(kernel, remainder_f,  "remainder_f(number a,number b) -> number");
        import_function(kernel, mod_i, "mod_i(int a,int b) -> int");
        import_function(kernel, mod_f, "mod_f(number a,number b) -> number");
        import_function(kernel, round, "round(number n) -> int;"
            "'Return the integer that is closest to n'");
        import_function(kernel, floor, "floor(number n) -> int;"
        "'Return the closest integer that is less than n'");
        import_function(kernel, ceil, "ceil(number n) -> int;"
        "'Return the closest integer that is greater than n'");
        import_function(kernel, average, "average(number vals :multiple) -> number;"
                "'Returns the average of all inputs.'");
        import_function(kernel, pow, "pow(int i, int x) -> int; 'Returns i to the power of x'");
        import_function(kernel, sqr, "sqr(number n) -> number; 'Square function'");
        import_function(kernel, cube, "cube(number n) -> number; 'Cube function'");
        import_function(kernel, sqrt, "sqrt(number n) -> number; 'Square root'");
        import_function(kernel, log, "log(number n) -> number; 'Natural log function'");
    }
}
} // namespace circa
