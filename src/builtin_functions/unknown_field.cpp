// Copyright 2009 Paul Hodge

#include "circa.h"

namespace circa {
namespace unknown_field_function {

    void evaluate(Term* caller)
    {
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate, "unknown_field(any...) : any");

        UNKNOWN_FIELD_FUNC = main_func;
    }
}
}
