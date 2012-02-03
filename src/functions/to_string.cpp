// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa_internal.h"

namespace circa {
namespace to_string_function {

    CA_FUNCTION(evaluate)
    {
        set_string(OUTPUT, to_string(INPUT(0)));
    }

    void setup(Branch* kernel)
    {
        import_function(kernel, evaluate, "to_string(any) -> string");
    }
}
} // namespace circa
