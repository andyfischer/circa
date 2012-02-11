// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace cast_function {

    CA_FUNCTION(cast_evaluate)
    {
        TValue* source = INPUT(0);

        if (CALLER->type == &ANY_T)
            return copy(source, OUTPUT);

        Type* type = CALLER->type;
        if (!cast_possible(source, type)) {
            std::stringstream message;
            message << "Can't cast value " << source->toString()
                << " to type " << name_to_string(type->name);
            return RAISE_ERROR(message.str().c_str());
        }

        //change_type(OUTPUT, type);
        bool success = cast(source, type, OUTPUT);

        if (!success)
            return RAISE_ERROR("cast failed");
    }

    void setup(Branch* kernel)
    {
        FUNCS.cast = import_function(kernel, cast_evaluate, "cast :throws (any) -> any");
    }
}
}
