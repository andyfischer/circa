// Copyright 2008 Andrew Fischer

#include <circa.h>

namespace circa {
namespace length_function {

    void evaluate(Term* caller)
    {
        as_int(caller) = as_branch(caller->input(0)).length();
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "length(List) : int");
    }
}
}
