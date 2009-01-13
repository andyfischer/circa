// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace equals_function {

    void evaluate(Term* caller)
    {
        Term *input0 = caller->input(0);
        Term *input1 = caller->input(1);
        if (input0->type != input1->type) {
            error_occured(caller, "different types");
            return;
        }

        Type &type = as_type(input0->type);

        if (type.equals == NULL) {
            std::stringstream error;
            error << "type " << type.name << " has no equals function";
            error_occured(caller, error.str());
            return;
        }

        as_bool(caller) = type.equals(input0, input1);
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate,
                "function equals(any,any) -> bool");
        as_function(main_func).pureFunction = true;
    }
}
}
