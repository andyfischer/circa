// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"

namespace circa {
namespace set_field_function {

    CA_FUNCTION(evaluate)
    {
        copy(INPUT(0), OUTPUT);
        touch(OUTPUT);
        const char* name = INPUT(1)->asCString();
        int index = list_find_field_index_by_name(OUTPUT->value_type, name);
        if (index == -1)
            return error_occurred(CONTEXT, CALLER, std::string("field not found: ") + name);
        copy(INPUT(2), OUTPUT->getIndex(index));
    }

    Type* specializeType(Term* caller)
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

    void setup(Branch* kernel)
    {
        SET_FIELD_FUNC = import_function(kernel, evaluate,
                "set_field(any, string, any) -> any");
        as_function(SET_FIELD_FUNC)->specializeType = specializeType;
        as_function(SET_FIELD_FUNC)->formatSource = formatSource;
    }
}
}
