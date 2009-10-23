// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

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

    void evaluate_f(Term* caller)
    {
        seed_if_needed();
        as_float(caller) = (float) rand() / RAND_MAX;
    }

    void evaluate_f_range(Term* caller)
    {
        seed_if_needed();
        float r = (float) rand() / RAND_MAX;
        float min = caller->input(0)->toFloat();
        float max = caller->input(1)->toFloat();

        if (min >= max) {
            error_occurred(caller, "min is >= max");
            return;
        }

        as_float(caller) = min + r * (max - min);
    }

    void evaluate_i(Term* caller)
    {
        seed_if_needed();

        as_int(caller) = rand();
    }

    void evaluate_i_i(Term* caller)
    {
        seed_if_needed();
        int period = caller->input(0)->asInt();

        // TODO: replace this, builtin rand() does not have good randomness in lower bits.
        as_int(caller) = rand() % period;
    }

    void setup(Branch& kernel)
    {
        Term* rand_term = create_overloaded_function(kernel, "rand");
        import_function_overload(rand_term, evaluate_f, "rand() :: number");
        import_function_overload(rand_term, evaluate_f_range, "rand(number min,number max)::number");
        import_function(kernel, evaluate_i, "rand_i() :: int");
        //import_function(kernel, evaluate_i_i, "rand_i(int) : int");
    }
}
} // namespace circa
