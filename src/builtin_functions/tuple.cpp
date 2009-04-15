// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace tuple_function {

    void evaluate(Term* caller)
    {
        as<RefList>(caller) = caller->inputs;
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate, "tuple(any) -> Tuple");
        as_function(main_func).pureFunction = true;
        as_function(main_func).variableArgs = true;
    }
}

} // namespace circa
