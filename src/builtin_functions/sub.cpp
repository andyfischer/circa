// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace sub_function {

    void evaluate_i(Term* caller)
    {
        as_int(caller) = int_input(caller,0) - int_input(caller,1);
    }

    void evaluate_f(Term* caller)
    {
        as_float(caller) = float_input(caller,0) - float_input(caller,1);
    }

    void setup(Branch& kernel)
    {
        SUB_FUNC = create_overloaded_function(kernel, "sub");

        Term* sub_i = import_function_overload(SUB_FUNC, evaluate_i, "sub_i(int,int):int");
        Term* sub_f = import_function_overload(SUB_FUNC, evaluate_f, "sub_f(number,number):number");

        kernel.bindName(sub_i, "sub_i");
        kernel.bindName(sub_f, "sub_f");
    }
}
} // namespace circa
