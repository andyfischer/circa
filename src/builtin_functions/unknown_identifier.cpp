// Copyright 2008 Andrew Fischer

#include "builtins.h"
#include "circa.h"

namespace circa {
namespace unknown_identifier_function {

    void evaluate(Term* caller)
    {
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate, "unknown_identifier() : any");

        UNKNOWN_IDENTIFIER_FUNC = main_func;
    }
}
}
