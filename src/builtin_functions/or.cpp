// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace or_function {

    void evaluate(Term* caller)
    {
        set_bool(caller, bool_input(caller,0) || bool_input(caller,1));
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "or(bool a, bool b) -> bool;"
            "'Return whether either a or b are true' end");
    }
}
} // namespace circa
