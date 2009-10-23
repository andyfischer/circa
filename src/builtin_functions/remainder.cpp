// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace remainder_function {

    void evaluate(Term* caller)
    {
        int a = caller->input(0)->asInt();
        int n = caller->input(1)->asInt();

        as_int(caller) = a % n;
    }

    void evaluate_f(Term* caller)
    {
        float a = caller->input(0)->toFloat();
        float n = caller->input(1)->toFloat();

        as_float(caller) = fmodf(a, n);
    }

    void setup(Branch& kernel)
    {
        Term* main = create_overloaded_function(kernel, "remainder");
        import_function_overload(main, evaluate, "remainder(int,int) :: int");
        import_function_overload(main, evaluate_f, "remainder(number,number) :: number");
    }
}
} // namespace circa
