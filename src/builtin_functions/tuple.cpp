// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace tuple_function {

    void evaluate(Term* caller)
    {
        as<ReferenceList>(caller) = caller->inputs;
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_c_function(kernel, evaluate,
                "function tuple(any) -> Tuple");
        as_function(main_func).pureFunction = true;
        as_function(main_func).variableArgs = true;
    }
}

} // namespace circa
