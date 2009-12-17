// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace path_function {

    void evaluate(Term* caller)
    {
        as_string(caller) =
            get_path_relative_to_source(caller, as_string(caller->input(0)));
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "def path(string) -> string");
    }
}
}
