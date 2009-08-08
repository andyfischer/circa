// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "circa.h"

namespace circa {
namespace greater_than_function {

    void evaluate_i(Term* caller)
    {
        as_bool(caller) = as_int(caller->input(0)) > as_int(caller->input(1));
    }

    void evaluate_f(Term* caller)
    {
        as_bool(caller) = to_float(caller->input(0)) > to_float(caller->input(1));
    }

    void setup(Branch& kernel)
    {
        Term* main = create_overloaded_function(kernel, "greater_than");
        import_function_overload(main, evaluate_f, "greater_than(int,int) : bool");
        import_function_overload(main, evaluate_f, "greater_than(float,float) : bool");
    }
}
}
