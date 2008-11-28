// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace assert_function {

    void evaluate(Term* caller)
    {
        if (!as_bool(caller->inputs[0])) {
            std::cout << "Assert failed: " << caller->name << std::endl;
        }
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_c_function(kernel, evaluate,
                "function assert(bool)");
        as_function(main_func).hasSideEffects = true;
    }
}
}
