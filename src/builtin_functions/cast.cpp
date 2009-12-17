// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace cast_function {

    void evaluate(Term* caller)
    {
        if (!value_fits_type(caller->input(0), caller->type, NULL)) {
            std::stringstream message;
            message << "Value of type " << caller->input(0)->type->name;
            message << " doesn't fit in type " << caller->type->name << ".";
            return error_occurred(caller, message.str());
        }
        
        assign_value(caller->input(0), caller);
    }

    void setup(Branch& kernel)
    {
        CAST_FUNC = import_function(kernel, evaluate, "cast(any) -> any");
    }
}
}
