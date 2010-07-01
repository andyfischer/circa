// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace path_function {

    CA_FUNCTION(evaluate)
    {
        set_str(OUTPUT,
            get_path_relative_to_source(CALLER, as_string(INPUT(0))));
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "def path(string) -> string");
    }
}
}
