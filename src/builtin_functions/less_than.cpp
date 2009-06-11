// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace less_than_function {

    void evaluate_i(Term* caller)
    {
        as_bool(caller) = as_int(caller->input(0)) < as_int(caller->input(1));
    }

    void evaluate_f(Term* caller)
    {
        as_bool(caller) = to_float(caller->input(0)) < to_float(caller->input(1));
    }

    void setup(Branch& kernel)
    {
        Term* main = create_overloaded_function(&kernel, "less_than");
        import_function_overload(main, evaluate_f, "less_than(int,int) : bool");
        import_function_overload(main, evaluate_f, "less_than(float,float) : bool");
    }
}
}
