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

    Term* specializeType(Term* caller)
    {
        Type& type = as_type(caller->input(0)->type);
        std::string& name = caller->input(1)->asString();

        if (type.findFieldIndex(name) != -1)
            return type[name]->type;

        return ANY_TYPE;
    }


    void setup(Branch& kernel)
    {
        GET_FIELD_FUNC = import_function(kernel, evaluate,
                "get_field_by_name(any, string) : any");
        function_t::get_specialize_type(GET_FIELD_FUNC) = specializeType;
    }
}
}
