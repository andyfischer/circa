// Copyright 2008 Paul Hodge

#include <circa.h>

// This function is used inside subroutines, as a placeholder for an incoming
// input value.

namespace circa {
namespace input_placeholder_function {

    void evaluate(Term* caller)
    {
    }

    void setup(Branch& kernel)
    {
        INPUT_PLACEHOLDER_FUNC = import_function(kernel, evaluate, "input_placeholder() : any");
    }
}
}
