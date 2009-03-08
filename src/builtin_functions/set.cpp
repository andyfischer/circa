// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace set_function {

    void evaluate(Term* caller)
    {
        Branch &result = as_branch(caller);
        result.clear();

        for (int index=0; index < caller->numInputs(); index++) {
            set_t::add(result, caller->input(index));
        }
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate,
                "function set(any) -> Set");
        as_function(main_func).pureFunction = true;
        as_function(main_func).variableArgs = true;
    }
}
}
