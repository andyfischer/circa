// Copyright 2008 Paul Hodge

#include "circa.h"
#include "set.h"

namespace circa {
namespace set_function {

    void evaluate(Term* caller)
    {
        Set &result = as<Set>(caller);
        result.clear();

        for (int index=0; index < caller->numInputs(); index++) {
            result.add(caller->input(index));
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
