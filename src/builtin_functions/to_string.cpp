// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace to_string_function {

    void evaluate(Term* caller)
    {
        Term* term = caller->input(0);
        as_string(caller) = to_string(term);
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "to_string(any) :: string");
    }
}
} // namespace circa
