// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "circa.h"

namespace circa {
namespace get_field_function {

    void evaluate(Term* caller)
    {
        std::string name = caller->input(1)->asString();

        int index = type_t::find_field_index(caller->input(0)->type, name);

        if (index == -1) {
            error_occurred(caller, "field not found: " + name);
            return;
        }

        Term* field = as_branch(caller->input(0))[index];
        assign_value(field, caller);
    }

    Term* specializeType(Term* caller)
    {
        std::string& name = caller->input(1)->asString();

        Branch& type_prototype = type_t::get_prototype(caller->input(0)->type);

        if (type_prototype.contains(name))
            return type_prototype[name]->type;

        return ANY_TYPE;
    }

    std::string toSourceString(Term* term)
    {
        std::stringstream out;
        prepend_name_binding(term, out);
        out << get_source_of_input(term, 0);
        out << "." << term->input(1)->asString();
        return out.str();
    }

    void setup(Branch& kernel)
    {
        GET_FIELD_FUNC = import_function(kernel, evaluate,
                "get_field_by_name(any, string) :: any");
        function_t::get_specialize_type(GET_FIELD_FUNC) = specializeType;
        function_t::get_to_source_string(GET_FIELD_FUNC) = toSourceString;
    }
}
}
