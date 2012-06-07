// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include <algorithm>

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace math_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(max_f,
            "max_f(number,number) -> number; 'Maximum of two numbers'")
    {
        set_float(OUTPUT, std::max(FLOAT_INPUT(0), FLOAT_INPUT(1)));
    }

    CA_DEFINE_FUNCTION(max_i,
            "max_i(int,int) -> int; 'Maximum of two integers'")
    {
        set_int(OUTPUT, std::max(INT_INPUT(0), INT_INPUT(1)));
    }

    CA_DEFINE_FUNCTION(min_f,
            "min_f(number,number) -> number; 'Minimum of two numbers'")
    {
        set_float(OUTPUT, std::min(FLOAT_INPUT(0), FLOAT_INPUT(1)));
    }

    CA_DEFINE_FUNCTION(min_i,
            "min_i(int,int) -> int; 'Minimum of two integers'")
    {
        set_int(OUTPUT, std::min(INT_INPUT(0), INT_INPUT(1)));
    }

    CA_DEFINE_FUNCTION(remainder_i, "remainder_i(int,int) -> int")
    {
        set_int(OUTPUT, INT_INPUT(0) % INT_INPUT(1));
    }

    CA_DEFINE_FUNCTION(remainder_f, "remainder_f(number,number) -> number")
    {
        set_float(OUTPUT, fmodf(FLOAT_INPUT(0), FLOAT_INPUT(1)));
    }

    // We compute mod() using floored division. This is different than C and many
    // C-like languages which use truncated division. See this page for an explanation
    // of the difference:
    // http://en.wikipedia.org/wiki/Modulo_operation
    //
    // For a function that works the same as C's modulo, use remainder() . The % operator
    // also uses remainder(), so that it works the same as C's % operator.

    CA_DEFINE_FUNCTION(mod_i, "mod_i(int,int) -> int")
    {
        int a = INT_INPUT(0);
        int n = INT_INPUT(1);

        int out = a % n;
        if (out < 0)
            out += n;

        set_int(OUTPUT, out);
    }

    CA_DEFINE_FUNCTION(mod_f, "mod_f(number,number) -> number")
    {
        float a = FLOAT_INPUT(0);
        float n = FLOAT_INPUT(1);

        float out = fmodf(a, n);

        if (out < 0)
            out += n;

        set_float(OUTPUT, out);
    }

    CA_DEFINE_FUNCTION(round, "round(number n) -> int;"
        "'Return the integer that is closest to n'")
    {
        float input = FLOAT_INPUT(0);
        if (input > 0.0)
            set_int(OUTPUT, int(input + 0.5));
        else
            set_int(OUTPUT, int(input - 0.5));
    }

    CA_DEFINE_FUNCTION(floor, "floor(number n) -> int;"
        "'Return the closest integer that is less than n'")
    {
        set_int(OUTPUT, (int) std::floor(FLOAT_INPUT(0)));
    }

    CA_DEFINE_FUNCTION(ceil, "ceil(number n) -> int;"
        "'Return the closest integer that is greater than n'")
    {
        set_int(OUTPUT, (int) std::ceil(FLOAT_INPUT(0)));
    }

    CA_DEFINE_FUNCTION(average, "average(number :multiple) -> number;"
                "'Returns the average of all inputs.'")
    {
        caValue* args = circa_input(STACK, 0);
        int count = circa_count(args);
        caValue* out = circa_output(STACK, 0);

        if (count == 0) {
            set_float(out, 0);
            return;
        }

        float sum = 0;
        for (int i=0; i < count; i++)
            sum += to_float(circa_index(args, i));

        set_float(out, sum / count);
    }

    CA_DEFINE_FUNCTION(pow, "pow(int i, int x) -> int; 'Returns i to the power of x'")
    {
        set_int(OUTPUT, (int) std::pow((float) INT_INPUT(0), INT_INPUT(1)));
    }

    CA_DEFINE_FUNCTION(sqr, "sqr(number) -> number; 'Square function'")
    {
        float in = FLOAT_INPUT(0);
        set_float(OUTPUT, in * in);
    }
    CA_DEFINE_FUNCTION(cube, "cube(number) -> number; 'Cube function'")
    {
        float in = FLOAT_INPUT(0);
        set_float(OUTPUT, in * in * in);
    }

    CA_DEFINE_FUNCTION(sqrt, "sqrt(number) -> number; 'Square root'")
    {
        set_float(OUTPUT, std::sqrt(FLOAT_INPUT(0)));
    }

    CA_DEFINE_FUNCTION(log, "log(number) -> number; 'Natural log function'")
    {
        set_float(OUTPUT, std::log(FLOAT_INPUT(0)));
    }

    void setup(Branch* kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
    }
}
} // namespace circa
