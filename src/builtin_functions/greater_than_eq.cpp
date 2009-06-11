// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace greater_than_eq_function {

    void evaluate_i(Term* caller)
    {
        as_bool(caller) = as_int(caller->input(0)) >= as_int(caller->input(1));
    }

    void evaluate_f(Term* caller)
    {
        as_bool(caller) = to_float(caller->input(0)) >= to_float(caller->input(1));
    }

    void setup(Branch& kernel)
    {
        Term* main = create_overloaded_function(&kernel, "greater_than_eq");
        import_function_overload(main, evaluate_f, "greater_than_eq(int,int) : bool");
        import_function_overload(main, evaluate_f, "greater_than_eq(float,float) : bool");
    }
}
}
