// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace div_function {

    void evaluate_f(Term* caller)
    {
        as_float(caller) = float_input(caller,0) / float_input(caller,1);
    }

    void evaluate_i(Term* caller)
    {
        as_int(caller) = int_input(caller,0) / int_input(caller,1);
    }

    void setup(Branch& kernel)
    {
        Term* div_f = import_function(kernel, evaluate_f, "div_f(number,number) -> number");

        DIV_FUNC = create_overloaded_function(kernel, "div");
        create_ref(as_branch(DIV_FUNC), div_f);

        import_function(kernel, evaluate_i, "div_i(int,int) -> int");
    }
}
} // namespace circa
