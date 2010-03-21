// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace cast_function {

    void evaluate(EvalContext* cxt, Term* caller)
    {
        TaggedValue* source = caller->input(0);

        if (caller->type == ANY_TYPE)
            return copy(source, caller);

        Type* type = &as_type(caller->type);
        if (!cast_possible(type, source)) {
            std::stringstream message;
            message << "Can't cast from type " << source->value_type->name
                << " to type " << type->name;
            return error_occurred(cxt, caller, message.str());
        }

        cast(type, source, caller);
    }

    void setup(Branch& kernel)
    {
        CAST_FUNC = import_function(kernel, evaluate, "cast(any) -> any");
    }
}
}
