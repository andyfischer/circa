// Copyright 2008 Paul Hodge

#include "circa.h"

#include "time.h"

namespace circa {
namespace rand_function {

    void evaluate(Term* caller)
    {
        static bool seeded = false;

        if (!seeded) {
            srand((unsigned) time(0));
            seeded = true;
        }

        as_float(caller) = (float) rand() / 0x8fffffff;
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "rand() : float");
    }
}
} // namespace circa
