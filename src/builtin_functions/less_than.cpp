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
        Term* lt_i = import_function_overload(main, evaluate_f, "less_than(int,int) : bool");
        as_function(lt_i).pureFunction = true;
        Term* lt_f = import_function_overload(main, evaluate_f, "less_than(float,float) : bool");
        as_function(lt_f).pureFunction = true;
    }
}
}
