// Copyright 2009 Paul Hodge

#include "circa.h"

namespace circa {
namespace get_field_by_name_function {

    void evaluate(Term* caller)
    {
        std::string name = as_string(caller->state);
        Term* field = as_branch(caller->input(0))[name];
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
