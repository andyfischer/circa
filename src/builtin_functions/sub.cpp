// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace sub_function {

    void evaluate_i(EvalContext*, Term* caller)
    {
        set_int(caller, int_input(caller,0) - int_input(caller,1));
    }

    void evaluate_f(EvalContext*, Term* caller)
    {
        set_float(caller, float_input(caller,0) - float_input(caller,1));
    }

    void setup(Branch& kernel)
    {
        Term* sub_i = import_function(kernel, evaluate_i, "sub_i(int,int) -> int");
        Term* sub_f = import_function(kernel, evaluate_f, "sub_f(number,number) -> number");

        SUB_FUNC = create_overloaded_function(kernel, "sub", RefList(sub_i, sub_f));
    }
}
} // namespace circa
