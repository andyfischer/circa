// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace set_field_function {

    void evaluate(EvalContext* cxt, Term* caller)
    {
        assign_value(caller->input(0), caller);

        Term* head = caller;

        for (int nameIndex=2; nameIndex < caller->numInputs(); nameIndex++) {
            std::string name = caller->input(nameIndex)->asString();
            int index = head->value_type->findFieldIndex(name);

            if (index == -1) {
                error_occurred(cxt, caller, "field not found: "+name);
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

    void formatSource(StyledSource* source, Term* term)
    {
        format_source_for_input(source, term, 0);
        for (int i=2; i < term->numInputs(); i++) {
            append_phrase(source, ".", term, phrase_type::UNDEFINED);
            append_phrase(source, term->input(i)->asString().c_str(),
                    term, phrase_type::UNDEFINED);
        }
        append_phrase(source, " =", term, phrase_type::UNDEFINED);
        format_source_for_input(source, term, 1);
    }

    void setup(Branch& kernel)
    {
        SET_FIELD_FUNC = import_function(kernel, evaluate,
                "set_field(any, any, string...) -> any");
        function_t::get_attrs(SET_FIELD_FUNC).specializeType = specializeType;
        function_t::get_attrs(SET_FIELD_FUNC).formatSource = formatSource;
    }
}
}
