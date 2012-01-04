// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"

namespace circa {
namespace unsafe_assign_function {

    CA_FUNCTION(evaluate)
    {
        // The thing we are changing is on the left, the desired value is on the right
        // This is a little confusing because the C function 'cast' is the other
        // way around. The reason we have this order is because the infix operator :=
        // arranges its inputs as destination := source.
        Term* destination = INPUT_TERM(0);
        Term* source = INPUT_TERM(1);

        if (!cast_possible(source, declared_type(destination))) {
            std::string msg = "Tried to assign a " + source->type->name + " to a "
                    + destination->type->name;
            ERROR_OCCURRED(msg.c_str());
            return;
        }

        cast(source, declared_type(destination), destination);
    }

    void setup(Branch* kernel)
    {
        FUNCS.unsafe_assign = import_function(kernel, evaluate, "unsafe_assign(any, any)");
    }
}
}
