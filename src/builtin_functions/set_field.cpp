// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace set_field_function {

    void evaluate(Term* caller)
    {
        assign_value(caller->input(0), caller);

        Term* head = caller;

        for (int nameIndex=2; nameIndex < caller->numInputs(); nameIndex++) {
            std::string name = caller->input(nameIndex)->asString();
            int index = type_t::find_field_index(head->type, name);

            if (index == -1) {
                error_occurred(caller, "field not found: "+name);
                return;
            }

            head = as_branch(head)[index];
        }

        assign_value(caller->input(1), head);
    }

    Term* specializeType(Term* caller)
    {
        return caller->input(0)->type;
    }

    std::string toSourceString(Term* term)
    {
        std::stringstream out;
        out << get_source_of_input(term, 0);
        for (int i=2; i < term->numInputs(); i++)
            out << "." << term->input(i)->asString();
        out << " =";
        out << get_source_of_input(term, 1);
        return out.str();
    }

    void setup(Branch& kernel)
    {
        SET_FIELD_FUNC = import_function(kernel, evaluate,
                "set_field(any, any, string...) -> any");
        function_t::get_specialize_type(SET_FIELD_FUNC) = specializeType;
        function_t::get_to_source_string(SET_FIELD_FUNC) = toSourceString;
    }
}
}
