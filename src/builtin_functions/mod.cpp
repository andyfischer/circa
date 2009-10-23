// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "circa.h"

namespace circa {
namespace mod_function {

    // We compute mod() using floored division. This is different than C and many
    // C-like languages. See this page for an explanation of the difference:
    // http://en.wikipedia.org/wiki/Modulo_operation
    //
    // For a function that works the same as C's modulo, use remainder()

    void evaluate(Term* caller)
    {
        int a = int_input(caller, 0);
        int n = int_input(caller, 1);

        if (a >= 0)
            as_int(caller) = a % n;
        else
            as_int(caller) = a % n + n;
    }

    void evaluate_f(Term* caller)
    {
        float a = float_input(caller, 0);
        float n = float_input(caller, 1);

        if (a >= 0)
            as_float(caller) = fmodf(a, n);
        else
            as_float(caller) = fmodf(a, n) + n;
    }

    void setup(Branch& kernel)
    {
        Term* main = create_overloaded_function(kernel, "mod");
        import_function_overload(main, evaluate, "mod(int,int) :: int");
        import_function_overload(main, evaluate_f, "mod(number,number) :: number");
    }
}
} // namespace circa
