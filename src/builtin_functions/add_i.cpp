// Copyright 2008 Paul Hodge

#include <circa.h>

namespace circa {
namespace add_i_function {

    void evaluate(Term* caller)
    {
        int result = 0;
        for (int i=0; i < caller->numInputs(); i++)
            result += as_int(caller->input(i));
        as_int(caller) = result;
    }

    void setup(Branch& kernel)
    {
        Term* func = import_function(kernel, evaluate, "add_i(int...) : int");
        as_function(func).pureFunction = true;
    }
}
}
