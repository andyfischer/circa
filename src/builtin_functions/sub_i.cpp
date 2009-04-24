// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace sub_i_function {

    void evaluate(Term* caller)
    {
        as_int(caller) = as_int(caller->input(0)) - as_int(caller->input(1));
    }

    void setup(Branch& kernel)
    {
        Term* func = import_function(kernel, evaluate, "sub_i(int,int) : int");
        as_function(func).pureFunction = true;
    }
}
} // namespace circa
