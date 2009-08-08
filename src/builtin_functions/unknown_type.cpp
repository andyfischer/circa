// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace unknown_type_function {

    void evaluate(Term* caller)
    {
        initialize_empty_type(caller);
    }

    void setup(Branch& kernel)
    {
        UNKNOWN_TYPE_FUNC = import_function(kernel, evaluate, "unknown_type() : Type");
    }
}
}
