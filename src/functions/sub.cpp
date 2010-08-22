// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace sub_function {

    CA_FUNCTION(evaluate_i)
    {
        make_int(OUTPUT, INT_INPUT(0) - INT_INPUT(1));
    }

    CA_FUNCTION(evaluate_f)
    {
        make_float(OUTPUT, FLOAT_INPUT(0) - FLOAT_INPUT(1));
    }

    void setup(Branch& kernel)
    {
        Term* sub_i = import_function(kernel, evaluate_i, "sub_i(int,int) -> int");
        Term* sub_f = import_function(kernel, evaluate_f, "sub_f(number,number) -> number");

        SUB_FUNC = create_overloaded_function(kernel, "sub", RefList(sub_i, sub_f));
    }
}
} // namespace circa
