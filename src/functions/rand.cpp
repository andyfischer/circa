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

    CA_FUNCTION(evaluate_f)
    {
        seed_if_needed();
        set_float(OUTPUT, (float) rand() / RAND_MAX);
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

    CA_FUNCTION(evaluate_i)
    {
        seed_if_needed();
        set_int(OUTPUT, rand());
    }

    CA_FUNCTION(evaluate_i_i)
    {
        seed_if_needed();
        int period = as_int(INPUT(0));

        // TODO: replace this, builtin rand() does not have good randomness in lower bits.
        set_int(OUTPUT, rand() % period);
    }

    void setup(Branch* kernel)
    {
        import_function(kernel, evaluate_f, "rand() -> number");
        import_function(kernel, evaluate_f_range,
                "rand_range(number min,number max) -> number");
        import_function(kernel, evaluate_i, "rand_i() -> int");
    }
}
} // namespace circa
