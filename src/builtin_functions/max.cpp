// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

#include <algorithm>

namespace circa {
namespace max_function {

    void evaluate_f(EvalContext*, Term* caller)
    {
        set_float(caller, std::max(float_input(caller,0), float_input(caller,1)));
    }

    void evaluate_i(EvalContext*, Term* caller)
    {
        set_int(caller, std::max(int_input(caller,0), int_input(caller,1)));
    }

    void setup(Branch& kernel)
    {
        Term* max_f = import_function(kernel, evaluate_f,
            "max_f(number,number) -> number; 'Maximum of two numbers' end");
        Term* max_i = import_function(kernel, evaluate_i,
            "max_i(int,int) -> int; 'Maximum of two integers' end");

        create_overloaded_function(kernel, "max", RefList(max_i, max_f));
    }
}
} // namespace circa
