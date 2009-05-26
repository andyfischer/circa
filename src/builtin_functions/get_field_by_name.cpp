// Copyright 2009 Paul Hodge

#include "circa.h"

namespace circa {
namespace get_field_by_name_function {

    void evaluate(Term* caller)
    {
        std::string name = caller->stringProp("field-name");
        int index = as_type(caller->input(0)->type).findFieldIndex(name);
        if (index == -1) {
            error_occurred(caller, "field not found: " + name);
            return;
        }
        Term* field = as_branch(caller->input(0))[index];
        assign_value(field, caller);
    }

    void setup(Branch& kernel)
    {
        GET_FIELD_BY_NAME_FUNC = import_function(kernel, evaluate,
                "get_field_by_name(any) : any");
        as_function(GET_FIELD_BY_NAME_FUNC).pureFunction = true;
    }
}
}
