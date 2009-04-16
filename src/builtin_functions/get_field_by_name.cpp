// Copyright 2009 Paul Hodge

#include "circa.h"

namespace circa {
namespace get_field_by_name_function {

    void evaluate(Term* caller)
    {
        std::string name = as_string(caller->state);
        int index = as_type(caller->input(0)->type).findFieldIndex(name);
        if (index == -1) {
            error_occured(caller, "field not found: " + name);
            return;
        }
        Term* field = as_branch(caller->input(0))[index];
        assign_value(field, caller);
    }

    void setup(Branch& kernel)
    {
        GET_FIELD_BY_NAME_FUNC = import_function(kernel, evaluate,
                "get_field_by_name(any) -> any");
        as_function(GET_FIELD_BY_NAME_FUNC).stateType = STRING_TYPE;
        as_function(GET_FIELD_BY_NAME_FUNC).pureFunction = true;
    }
}
}
