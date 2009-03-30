// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace set_field_function {

    void evaluate(Term* caller)
    {
        int index = as_int(caller->state);
        assign_value(caller->input(0), caller);

        assign_value(caller->input(1), as_branch(caller)[index]);
    }

    Term* specializeType(Term* caller)
    {
        return caller->input(0)->type;
    }

    void setup(Branch& kernel)
    {
        SET_FIELD_FUNC = import_function(kernel, evaluate,
                "function set-field(any, any) -> any");
        as_function(SET_FIELD_FUNC).stateType = INT_TYPE;
        as_function(SET_FIELD_FUNC).specializeType = specializeType;
        as_function(SET_FIELD_FUNC).pureFunction = true;
    }
}
}
