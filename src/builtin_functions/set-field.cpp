// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace set_field_function {

    void evaluate(Term* caller)
    {
        std::string fieldName = as_string(caller->input(1));

        recycle_value(caller->input(0), caller);

        recycle_value(caller->input(2), as_branch(caller)[fieldName]);
    }

    Term* specializeType(Term* caller)
    {
        return caller->input(0)->type;
    }

    void setup(Branch& kernel)
    {
        SET_FIELD_FUNC = import_function(kernel, evaluate,
                "function set-field(any, any) -> any");
        as_function(SET_FIELD_FUNC).specializeType = specializeType;
        as_function(SET_FIELD_FUNC).pureFunction = true;
    }
}
}
