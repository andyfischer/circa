// Copyright 2008 Andrew Fischer

#include <circa.h>

namespace circa {
namespace and_function {

    void evaluate(Term* caller)
    {
        as_bool(caller) = as_bool(caller->input(0)) && as_bool(caller->input(1));
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "and(bool,bool) : bool");
    }
}
}
