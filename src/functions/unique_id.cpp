// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace unique_id_function {

    CA_FUNCTION(evaluate)
    {
        static int nextId = 1;
        set_int(OUTPUT, nextId++);
    }

    void setup(Block* kernel)
    {
        import_function(kernel, evaluate, "unique_id() -> int");
    }
}
}
