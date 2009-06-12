// Copyright 2009 Paul Hodge

#include "circa.h"

namespace circa {
namespace get_field_function {

    void evaluate(Term* caller)
    {
        std::string name = caller->input(1)->asString();

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
        GET_FIELD_FUNC = import_function(kernel, evaluate,
                "get_field_by_name(any, string) : any");
    }
}
}
