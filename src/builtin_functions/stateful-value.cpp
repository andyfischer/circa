// Copyright 2008 Paul Hodge

#include "circa.h"

#define EXTENDED_LOGGING 0

namespace circa {
namespace stateful_value_function {

    void evaluate(Term* caller)
    {
        if (EXTENDED_LOGGING)
            std::cout << "stateful-value eval" << std::endl;

        if (EXTENDED_LOGGING)
            if (caller->name == "") std::cout << "caller has no name" << std::endl;

        // Copy our value from the bottom of this branch, if it is ready.
        Term* bottom = caller->owningBranch->getNamed(caller->name);

        if (bottom == caller) {
            if (EXTENDED_LOGGING)
                std::cout << "bottom is same for:" << caller->name << std::endl;
            return;
        }

        if (!caller->hasValue()) {
            if (EXTENDED_LOGGING)
                std::cout << "caller has no value" << std::endl;
            return;
        }

        // Temp, ignore terms with wrong type. This check should be
        // removed when we have proper type specialization.
        if (bottom->type != caller->type) {
            if (EXTENDED_LOGGING)
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
