// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace mod_function {

    // We compute mod() using floored division. This is different than C and many
    // C-like languages. See this page for an explanation of the difference:
    // http://en.wikipedia.org/wiki/Modulo_operation
    //
    // For a function that works the same as C's modulo, use remainder()

    void evaluate_i(EvalContext*, Term* caller)
    {
        int a = int_input(caller, 0);
        int n = int_input(caller, 1);

        if (a >= 0)
            set_int(caller, a % n);
        else
            set_int(caller, a % n + n);
    }

    void evaluate_f(EvalContext*, Term* caller)
    {
        float a = float_input(caller, 0);
        float n = float_input(caller, 1);

        if (a >= 0)
            set_float(caller, fmodf(a, n));
        else
            set_float(caller, fmodf(a, n) + n);
    }

    void setup(Branch& kernel)
    {
        Term* mod_i = import_function(kernel, evaluate_i, "mod_i(int,int) -> int");
        Term* mod_f = import_function(kernel, evaluate_f, "mod_f(number,number) -> number");

        create_overloaded_function(kernel, "mod", RefList(mod_i, mod_f));
    }
}
} // namespace circa
