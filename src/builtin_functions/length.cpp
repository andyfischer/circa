// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace length_function {

    void evaluate(EvalContext*, Term* caller)
    {
        set_int(caller, num_elements(caller->input(0)));
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "length(List) -> int;"
            "'Return the number of items in the given list' end");
    }
}
}
