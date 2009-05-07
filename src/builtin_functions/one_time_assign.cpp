// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace one_time_assign_function {

    void evaluate(Term* caller)
    {
        bool &assigned = caller->input(0)->asBool();

        if (!assigned) {
            assign_value(caller->input(1), caller);
            assigned = true;
        }
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate,
                "one_time_assign(bool, any) : any");
    }
}
}
