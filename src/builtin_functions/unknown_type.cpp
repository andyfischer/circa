// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace unknown_type_function {

    void evaluate(Term* caller)
    {
        setup_empty_type(as_type(caller));
    }

    void setup(Branch& kernel)
    {
        UNKNOWN_TYPE_FUNC = import_function(kernel, evaluate, "unknown_type() : Type");
    }
}
}
