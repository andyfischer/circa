// Copyright 2008 Paul Hodge

#include "circa.h"

#define EXTENDED_LOGGING false

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

        if (!bottom->hasValue()) {
            if (EXTENDED_LOGGING)
                std::cout << "bottom has no value" << std::endl;
            return;
        }

        if (bottom->needsUpdate) {
            if (EXTENDED_LOGGING)
                std::cout << "bottom needs update" << std::endl;
            return;
        }

        // Temp, ignore terms with wrong type. This check should be
        // removed when we have proper type specialization.
        if (bottom->type != caller->type) {
            if (EXTENDED_LOGGING)
                std::cout << "type doesn't match for: " << caller->name << std::endl;
            return;
        }

        copy_value(bottom, caller);
    }

    void setup(Branch& kernel)
    {
        STATEFUL_VALUE_FUNC = import_function(kernel, evaluate, "stateful-value() -> any");
        as_function(STATEFUL_VALUE_FUNC).stateType = ANY_TYPE; // state contains initial value
    }
}
}
