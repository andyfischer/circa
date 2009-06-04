// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace sub_function {

    void evaluate_i(Term* caller)
    {
        as_int(caller) = as_int(caller->input(0)) - as_int(caller->input(1));
    }

    void evaluate_f(Term* caller)
    {
        as_float(caller) = to_float(caller->input(0)) - to_float(caller->input(1));
    }

    void setup(Branch& kernel)
    {
        SUB_FUNC = create_overloaded_function(&kernel, "sub");

        Term* sub_i = import_function_overload(SUB_FUNC, evaluate_i, "sub_i(int,int):int");
        as_function(sub_i).pureFunction = true;
        Term* sub_f = import_function_overload(SUB_FUNC, evaluate_f, "sub_f(float,float):float");
        as_function(sub_f).pureFunction = true;

        kernel.bindName(sub_i, "sub_i");
        kernel.bindName(sub_f, "sub_f");
    }
}
} // namespace circa
