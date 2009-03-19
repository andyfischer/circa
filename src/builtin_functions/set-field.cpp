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

    void setup(Branch& kernel)
    {
        SET_FIELD_FUNC = import_function(kernel, evaluate,
                "function set-field(List, string, any) -> List");
        as_function(SET_FIELD_FUNC).pureFunction = true;
    }
}
}
