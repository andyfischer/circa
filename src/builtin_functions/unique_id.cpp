// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace unique_id_function {

    CA_FUNCTION(evaluate)
    {
        static int nextId = 1;
        set_int(OUTPUT, nextId++);
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "unique_id() -> int");
    }
}
}
