// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace set_field_function {

    void evaluate(Term* caller)
    {
        assign_value(caller->input(0), caller);

        std::string name = caller->input(1)->asString();
        int index = as_type(caller->input(0)->type).findFieldIndex(name);
        assign_value(caller->input(2), as_branch(caller)[index]);
    }

    Term* specializeType(Term* caller)
    {
        return caller->input(0)->type;
    }

    void setup(Branch& kernel)
    {
        SET_FIELD_FUNC = import_function(kernel, evaluate,
                "set_field(any, string, any) : any");
        function_get_specialize_type(SET_FIELD_FUNC) = specializeType;
    }
}
}
