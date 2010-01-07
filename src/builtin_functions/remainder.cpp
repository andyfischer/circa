// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

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
        Term* rem_i = import_function(kernel, evaluate, "remainder_i(int,int) -> int");
        Term* rem_f = import_function(kernel, evaluate_f, "remainder_f(number,number) -> number");

        Term* main = create_overloaded_function(kernel, "remainder");
        create_ref(as_branch(main), rem_i);
        create_ref(as_branch(main), rem_f);
    }
}
} // namespace circa
