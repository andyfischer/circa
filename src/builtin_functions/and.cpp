// Copyright 2008 Paul Hodge

#include <circa.h>

namespace circa {
namespace and_function {

    void evaluate(Term* caller)
    {
        as_bool(caller) = as_bool(caller->input(0)) && as_bool(caller->input(1));
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate, "and(bool,bool) : bool");
        as_function(main_func).pureFunction = true;
    }
}
}
