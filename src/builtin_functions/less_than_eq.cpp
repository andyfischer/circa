// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace less_than_eq_function {

    void evaluate_i(Term* caller)
    {
        as_bool(caller) = int_input(caller,0) <= int_input(caller,1);
    }

    void evaluate_f(Term* caller)
    {
        as_bool(caller) = float_input(caller,0) <= float_input(caller,1);
    }

    void setup(Branch& kernel)
    {
        Term* lt_i = import_function(kernel, evaluate_i, "less_than_eq_i(int,int) -> bool");
        Term* lt_f = import_function(kernel, evaluate_f, "less_than_eq_f(number,number) -> bool");

        Term* main = create_overloaded_function(kernel, "less_than_eq");

        create_ref(as_branch(main), lt_i);
        create_ref(as_branch(main), lt_f);
    }
}
}
