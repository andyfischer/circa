// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace cast_function {

    CA_FUNCTION(cast_evaluate)
    {
        caValue* source = INPUT(0);

        if (CALLER->type == TYPES.any)
            return copy(source, OUTPUT);

        Type* type = CALLER->type;
        if (!cast_possible(source, type)) {
            std::stringstream message;
            message << "Can't cast value " << to_string(source)
                << " to type " << as_cstring(&type->name);
            return RAISE_ERROR(message.str().c_str());
        }

        //change_type(OUTPUT, type);
        copy(source, OUTPUT);
        bool success = cast(OUTPUT, type);

        if (!success)
            return RAISE_ERROR("cast failed");
    }

    void setup(Block* kernel)
    {
        FUNCS.cast = import_function(kernel, cast_evaluate, "cast :throws (any) -> any");
    }
}
}
