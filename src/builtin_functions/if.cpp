// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace if_function {

    void evaluate(Term* caller)
    {
        bool cond = as_bool(caller->input(0));

        if (cond) {
            Branch& branch = as_branch(caller->state);
            evaluate_branch(branch);
        }
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "function if(bool)");
    }
}
}
