// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace div_function {

    void evaluate_f(Term* caller)
    {
        as_float(caller) = to_float(caller->input(0)) / to_float(caller->input(1));
    }

    void evaluate_i(Term* caller)
    {
        as_int(caller) = as_int(caller->input(0)) / as_int(caller->input(1));
    }

    void setup(Branch& kernel)
    {
        DIV_FUNC = create_overloaded_function(&kernel, "div");

        Term* div_i = import_function_overload(DIV_FUNC, evaluate_i, "div_i(int,int) : int");
        Term* div_f = import_function_overload(DIV_FUNC, evaluate_f, "div_f(float,float) : float");

        kernel.bindName(div_f, "div_f");
        kernel.bindName(div_i, "div_i");
        as_function(div_f).pureFunction = true;
        as_function(div_i).pureFunction = true;
    }
}
} // namespace circa
