// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"

namespace circa {
namespace cast_function {

    CA_FUNCTION(cast_evaluate)
    {
        TaggedValue* source = INPUT(0);

        if (CALLER->type == &ANY_T)
            return copy(source, OUTPUT);

        Type* type = CALLER->type;
        if (!cast_possible(source, type)) {
            std::stringstream message;
            message << "Can't cast value " << source->toString()
                << " to type " << type->name;
            return ERROR_OCCURRED(message.str().c_str());
        }

        //change_type(OUTPUT, type);
        bool success = cast(source, type, OUTPUT);

        if (!success)
            return ERROR_OCCURRED("cast failed");
    }

    void setup(Branch* kernel)
    {
        CAST_FUNC = import_function(kernel, cast_evaluate, "cast :throws (any) -> any");
    }
}
}
