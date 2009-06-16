// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace do_once_function {

    void evaluate(Term* caller)
    {
        bool &done = caller->input(0)->asBool();

        if (!done) {
            evaluate_branch(caller->asBranch());
            done = true;
        }
    }

    void setup(Branch& kernel)
    {
        DO_ONCE_FUNC = import_function(kernel, evaluate, "do_once(state bool) : Branch");
    }
}
}
