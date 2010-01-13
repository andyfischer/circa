// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace sub_function {

    void evaluate_i(Term* caller)
    {
        set_value_int(caller, int_input(caller,0) - int_input(caller,1));
    }

    void evaluate_f(Term* caller)
    {
        set_value_float(caller, float_input(caller,0) - float_input(caller,1));
    }

    void setup(Branch& kernel)
    {
        Term* sub_i = import_function(kernel, evaluate_i, "sub_i(int,int) -> int");
        Term* sub_f = import_function(kernel, evaluate_f, "sub_f(number,number) -> number");

        SUB_FUNC = create_overloaded_function(kernel, "sub");

        create_ref(as_branch(SUB_FUNC), sub_i);
        create_ref(as_branch(SUB_FUNC), sub_f);
    }
}
} // namespace circa
