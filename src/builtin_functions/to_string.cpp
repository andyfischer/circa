// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace to_string_function {

    void evaluate(EvalContext*, Term* caller)
    {
        Term* term = caller->input(0);
        set_str(caller, to_string(term));
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "to_string(any) -> string");
    }
}
} // namespace circa
