// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace cast_function {

    CA_FUNCTION(cast_evaluate)
    {
        TaggedValue* source = INPUT(0);

        if (CALLER->type == ANY_TYPE)
            return copy(source, OUTPUT);

        Type* type = &as_type(CALLER->type);
        if (!value_fits_type(source, type)) {
            std::stringstream message;
            message << "Can't cast from type " << source->value_type->name
                << " to type " << type->name;
            return error_occurred(CONTEXT, CALLER, message.str());
        }

        cast(type, source, OUTPUT);
    }

    void setup(Branch& kernel)
    {
        CAST_FUNC = import_function(kernel, cast_evaluate, "cast(any) -> any");
    }
}
}
