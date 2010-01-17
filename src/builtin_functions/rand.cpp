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

    void evaluate_f(Term* caller)
    {
        seed_if_needed();
        set_float(caller, (float) rand() / RAND_MAX);
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

        set_float(caller, min + r * (max - min));
    }

    void evaluate_i(Term* caller)
    {
        seed_if_needed();

        set_int(caller, rand());
    }

    void evaluate_i_i(Term* caller)
    {
        seed_if_needed();
        int period = caller->input(0)->asInt();

        // TODO: replace this, builtin rand() does not have good randomness in lower bits.
        set_int(caller, rand() % period);
    }

    void setup(Branch& kernel)
    {
        Term* rand_f = import_function(kernel, evaluate_f, "rand_f() -> number");
        Term* rand_range = import_function(kernel, evaluate_f_range,
                "rand_range(number min,number max) -> number");
        import_function(kernel, evaluate_i, "rand_i() -> int");
        //import_function(kernel, evaluate_i_i, "rand_i(int) : int");
        
        Term* rand_func = create_overloaded_function(kernel, "rand");

        create_ref(as_branch(rand_func), rand_f);
        create_ref(as_branch(rand_func), rand_range);

    }
}
} // namespace circa
