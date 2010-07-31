// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace to_string_function {

    CA_FUNCTION(evaluate)
    {
        set_str(OUTPUT, to_string(INPUT(0)));
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "to_string(any) -> string");
    }
}
} // namespace circa
