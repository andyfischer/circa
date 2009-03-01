// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace for_function {

    void evaluate(Term* caller)
    {
        Branch& inner = as_branch(caller);


    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate, "function for()");
        as_function(main_func).pureFunction = true;
    }
}
} // namespace circa
