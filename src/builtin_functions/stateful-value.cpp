// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace stateful_value_function {

    void evaluate(Term* caller)
    {
        // Copy our value from the bottom of this branch, if it is ready.
        Term* bottom = caller->owningBranch->getNamed(caller->name);
        if (bottom == caller)
            return;

        if (!caller->hasValue())
            return;

        copy_value(bottom, caller);
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate,
            "stateful-value() -> any");
    }
}
}
