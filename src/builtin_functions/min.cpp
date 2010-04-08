// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

#include <algorithm>

namespace circa {
namespace min_function {

    void evaluate_f(EvalContext*, Term* caller)
    {
        set_float(caller, std::min(float_input(caller,0), float_input(caller,1)));
    }
    void evaluate_i(EvalContext*, Term* caller)
    {
        set_float(caller, std::min(float_input(caller,0), float_input(caller,1)));
    }

    void setup(Branch& kernel)
    {
        Term* min_f = import_function(kernel, evaluate_f,
                "min_f(number,number) -> number; 'Minimum of two numbers.' end");
        Term* min_i = import_function(kernel, evaluate_i,
                "min_i(int,int) -> int; 'Minimum of two integers.' end");
        create_overloaded_function(kernel, "min", RefList(min_i, min_f));
    }
}
} // namespace circa
