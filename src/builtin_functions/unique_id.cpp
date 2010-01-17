// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace unique_id_function {

    void evaluate(Term* caller)
    {
        static int nextId = 1;
        set_int(caller, nextId++);
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "unique_id() -> int");
    }
}
}
