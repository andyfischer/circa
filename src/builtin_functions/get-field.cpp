// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace get_field_function {

    void evaluate(Term* caller)
    {
        std::string fieldName = as_string(caller->input(1));
        Term* field = as_branch(caller->input(0))[fieldName];
        specialize_type(caller, field->type);

        recycle_value(field, caller);
    }

    void setup(Branch& kernel)
    {
        GET_FIELD_FUNC = import_function(kernel, evaluate,
                "function get-field(any,string) -> any");
        as_function(GET_FIELD_FUNC).pureFunction = true;
    }
}
}
