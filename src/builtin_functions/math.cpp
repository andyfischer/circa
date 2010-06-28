// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"
#include "importing_macros.h"

#include <algorithm>

namespace circa {
namespace math_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(max_f,
            "max_f(number,number) -> number; 'Maximum of two numbers' end")
    {
        set_float(OUTPUT, std::max(FLOAT_INPUT(0), FLOAT_INPUT(1)));
    }

    CA_DEFINE_FUNCTION(max_i,
            "max_i(int,int) -> int; 'Maximum of two integers' end")
    {
        set_int(OUTPUT, std::max(INT_INPUT(0), INT_INPUT(1)));
    }

    CA_DEFINE_FUNCTION(min_f,
            "min_f(number,number) -> number; 'Minimum of two numbers' end")
    {
        set_float(OUTPUT, std::min(FLOAT_INPUT(0), FLOAT_INPUT(1)));
    }

    CA_DEFINE_FUNCTION(min_i,
            "min_i(int,int) -> int; 'Minimum of two integers' end")
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
    // C-like languages. See this page for an explanation of the difference:
    // http://en.wikipedia.org/wiki/Modulo_operation
    //
    // For a function that works the same as C's modulo, use remainder()

    CA_DEFINE_FUNCTION(mod_i, "mod_i(int,int) -> int")
    {
        int a = INT_INPUT(0);
        int n = INT_INPUT(1);

        if (a >= 0)
            set_int(OUTPUT, a % n);
        else
            set_int(OUTPUT, a % n + n);
    }

    CA_DEFINE_FUNCTION(mod_f, "mod_f(number,number) -> number")
    {
        float a = FLOAT_INPUT(0);
        float n = FLOAT_INPUT(1);

        if (a >= 0)
            set_float(OUTPUT, fmodf(a, n));
        else
            set_float(OUTPUT, fmodf(a, n) + n);
    }

    CA_DEFINE_FUNCTION(round, "round(number n) -> int;"
        "'Return the integer that is closest to n' end")
    {
        float input = FLOAT_INPUT(0);
        if (input > 0.0)
            set_int(OUTPUT, int(input + 0.5));
        else
            set_int(OUTPUT, int(input - 0.5));
    }

    CA_DEFINE_FUNCTION(floor, "floor(number n) -> int;"
        "'Return the closest integer that is less than n' end")
    {
        set_int(OUTPUT, (int) std::floor(FLOAT_INPUT(0)));
    }

    CA_DEFINE_FUNCTION(ceil, "ceil(number n) -> int;"
        "'Return the closest integer that is greater than n' end")
    {
        set_int(OUTPUT, (int) std::ceil(FLOAT_INPUT(0)));
    }

    CA_DEFINE_FUNCTION(average, "average(number...) -> number;"
                "'Returns the average of all inputs.' end")
    {
        if (NUM_INPUTS == 0) {
            set_float(OUTPUT, 0);
            return;
        }

        float sum = 0;
        for (int i=0; i < NUM_INPUTS; i++)
            sum += FLOAT_INPUT(i);

        set_float(OUTPUT, sum / NUM_INPUTS);
    }

    CA_DEFINE_FUNCTION(pow, "pow(int i, int x) -> int; 'Returns i to the power of x' end")
    {
        set_int(OUTPUT, (int) std::pow((float) INT_INPUT(0), INT_INPUT(1)));
    }

    CA_DEFINE_FUNCTION(sqr, "sqr(number) -> number; 'Square function' end")
    {
        float in = FLOAT_INPUT(0);
        set_float(OUTPUT, in * in);
    }

    CA_DEFINE_FUNCTION(sqrt, "sqrt(number) -> number; 'Square root' end")
    {
        set_float(OUTPUT, std::sqrt(FLOAT_INPUT(0)));
    }

    void setup(Branch& kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);

        create_overloaded_function(kernel, "max",
                RefList(kernel["max_i"], kernel["max_f"]));
        create_overloaded_function(kernel, "min",
                RefList(kernel["min_i"], kernel["min_f"]));
        create_overloaded_function(kernel, "remainder",
                RefList(kernel["remainder_i"], kernel["remainder_f"]));

        create_overloaded_function(kernel, "mod",
                RefList(kernel["mod_i"], kernel["mod_f"]));
    }
}
} // namespace circa
