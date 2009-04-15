// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace set_field_by_name_function {

    void evaluate(Term* caller)
    {
        std::string name = as_string(caller->state);
        assign_value(caller->input(0), caller);
        assign_value(caller->input(1), as_branch(caller)[name]);
    }

    Term* specializeType(Term* caller)
    {
        return caller->input(0)->type;
    }

    void setup(Branch& kernel)
    {
        SET_FIELD_BY_NAME_FUNC = import_function(kernel, evaluate,
                "set_field_by_name(any, any) -> any");
        as_function(SET_FIELD_BY_NAME_FUNC).stateType = STRING_TYPE;
        as_function(SET_FIELD_BY_NAME_FUNC).specializeType = specializeType;
        as_function(SET_FIELD_BY_NAME_FUNC).pureFunction = true;
    }
}
}
