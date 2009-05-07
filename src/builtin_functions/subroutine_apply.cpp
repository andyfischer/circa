// Copyright 2008 Paul Hodge

#include "circa.h"
#include "function.h"

namespace circa {
namespace subroutine_apply_function {

    void evaluate(Term* caller)
    {
        assign_value(caller->input(0), caller);
        std::string input = as_string(caller->input(1));

        Function& sub = as_function(caller);

        sub.subroutineBranch.compile(input);
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate,
            "subroutine_apply(Function, string) : Function");
    }
}
}
