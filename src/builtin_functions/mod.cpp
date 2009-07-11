// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace mod_function {

    void evaluate(Term* caller)
    {
        as_int(caller) = as_int(caller->input(0)) % as_int(caller->input(1));
    }

    void evaluate_f(Term* caller)
    {
        as_float(caller) = fmodf(caller->input(0)->toFloat(), caller->input(1)->toFloat());
    }

    void setup(Branch& kernel)
    {
        Term* main = create_overloaded_function(kernel, "mod");
        import_function_overload(main, evaluate, "mod(int,int) : int");
        import_function_overload(main, evaluate_f, "mod(float,float) : float");
    }
}
} // namespace circa
