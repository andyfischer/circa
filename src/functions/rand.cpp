// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

#include "time.h"

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
        make_float(CALLER, (float) rand() / RAND_MAX);
    }

    CA_FUNCTION(evaluate_f_range)
    {
        seed_if_needed();
        float r = (float) rand() / RAND_MAX;
        float min = INPUT(0)->toFloat();
        float max = INPUT(1)->toFloat();

        if (min >= max) {
            error_occurred(CONTEXT, CALLER, "min is >= max");
            return;
        }

        make_float(CALLER, min + r * (max - min));
    }

    CA_FUNCTION(evaluate_i)
    {
        seed_if_needed();
        make_int(OUTPUT, rand());
    }

    CA_FUNCTION(evaluate_i_i)
    {
        seed_if_needed();
        int period = INPUT(0)->asInt();

        // TODO: replace this, builtin rand() does not have good randomness in lower bits.
        make_int(OUTPUT, rand() % period);
    }

    void setup(Branch& kernel)
    {
        Term* rand_f = import_function(kernel, evaluate_f, "rand_f() -> number");
        Term* rand_range = import_function(kernel, evaluate_f_range,
                "rand_range(number min,number max) -> number");
        import_function(kernel, evaluate_i, "rand_i() -> int");
        //import_function(kernel, evaluate_i_i, "rand_i(int) : int");
        
        create_overloaded_function(kernel, "rand", RefList(rand_f, rand_range));
    }
}
} // namespace circa
