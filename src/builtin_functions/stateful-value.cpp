// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace stateful_value_function {

    void evaluate(Term* caller)
    {
        // Copy our value from the bottom of this branch, if it is ready.
        Term* bottom = caller->owningBranch->getNamed(caller->name);

        if (bottom == caller) {
            //std::cout << "bottom is same" << std::endl;
            return;
        }

        if (!caller->hasValue()) {
            std::cout << "caller has no value" << std::endl;
            return;
        }

        // Temp, ignore terms with wrong type. This check should be
        // removed when we have proper type specialization.
        if (bottom->type != caller->type) {
            std::cout << "type doesn't match" << std::endl;
            return;
        }

        copy_value(bottom, caller);
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "stateful-value() -> any");
    }
}
}
