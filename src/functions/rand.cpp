// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include <ctime>

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace rand_function {

    bool seeded = false;

    void seed_if_needed()
    {
        if (!seeded) {
            srand((unsigned) time(0));
            seeded = true;
        }
    }

    void evaluate_f(caStack* stack)
    {
        seed_if_needed();
        set_float(circa_output(stack, 0), (float) rand() / RAND_MAX);
    }

    void evaluate_f_range(caStack* stack)
    {
        seed_if_needed();
        float r = (float) rand() / RAND_MAX;
        float min = circa_float_input(stack, 0);
        float max = circa_float_input(stack, 1);

        if (min >= max) {
            circa_output_error(stack, "min is >= max");
            return;
        }

        set_float(circa_output(stack, 0), min + r * (max - min));
    }

    void evaluate_i(caStack* stack)
    {
        seed_if_needed();
        int range = as_int(circa_input(stack, 0));

        // TODO: replace this, builtin rand() does not have good randomness in lower bits.
        set_int(circa_output(stack, 0), rand() % range);
    }

    void setup(Block* kernel)
    {
        import_function(kernel, evaluate_f, "rand() -> number\n -- Return a random number between 0.0 and 1.0, with a linear distribution.");
        import_function(kernel, evaluate_f_range,
                "rand_range(number min,number max) -> number\n -- Return a random number between the given min and max, with a linear distribution.");
        import_function(kernel, evaluate_i, "rand_i(int range) -> int\n -- Returns a random integer (with a linear distribution) that is equal to or greater than zero, and less than 'range'. ");
    }
}
} // namespace circa
