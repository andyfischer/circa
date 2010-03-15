// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace cast_function {

    void evaluate(EvalContext* cxt, Term* caller)
    {
#if 1
        // Old version

        if (!value_fits_type(caller->input(0), caller->type, NULL)) {
            std::stringstream message;
            message << "Value of type " << caller->input(0)->type->name;
            message << " doesn't fit in type " << caller->type->name << ".";
            return error_occurred(cxt, caller, message.str());
        }
        
        assign_value(caller->input(0), caller);
#endif
#if 0
        // New version
        Type* type = &as_type(caller->type);
        TaggedValue* source = caller->input(0);
        if (!cast_possible(type, source)) {
            std::stringstream message;
            message << "Can't cast from type " << source->value_type->name
                << " to type " << type->name;
            return error_occurred(cxt, caller, message.str());
        }

        cast(type, source, caller);
#endif
        
    }

    void setup(Branch& kernel)
    {
        CAST_FUNC = import_function(kernel, evaluate, "cast(any) -> any");
    }
}
}
