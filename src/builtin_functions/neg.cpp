// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace neg_function {

    void evaluate_f(Term* caller)
    {
        as_float(caller) = -to_float(caller->input(0));
    }

    void evaluate_i(Term* caller)
    {
        as_int(caller) = -as_int(caller->input(0));
    }

    void setup(Branch& kernel)
    {
        NEG_FUNC = create_overloaded_function(kernel, "neg");

        import_function_overload(NEG_FUNC, evaluate_i, "neg_i(int):int");
        import_function_overload(NEG_FUNC, evaluate_f, "neg_f(float):float");
    }
}
}
