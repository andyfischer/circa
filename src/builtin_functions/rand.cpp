// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

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

    void evaluate(Term* caller)
    {
        seed_if_needed();
        as_float(caller) = (float) rand() / RAND_MAX;
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
        import_function(kernel, evaluate, "rand() : float");
        import_function(kernel, evaluate_i, "rand_i() : int");
        //import_function(kernel, evaluate_i_i, "rand_i(int) : int");
    }
}
} // namespace circa
