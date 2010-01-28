// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace typeof_function {

    void evaluate(Term* caller)
    {
        //caller->value = caller->input(0)->type->value;
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "typeof(any) -> Type");
    }
}
}
