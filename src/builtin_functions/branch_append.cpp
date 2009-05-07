// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace branch_append_function {

    void evaluate(Term* caller)
    {
        assign_value(caller->input(0), caller);
        create_duplicate(&as_branch(caller), caller->input(1));
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate, "branch_append(Branch, any) : Branch");
        as_function(main_func).pureFunction = true;
    }
}
}
