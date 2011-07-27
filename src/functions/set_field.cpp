// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"

namespace circa {
namespace set_field_function {

    CA_FUNCTION(evaluate)
    {
        copy(INPUT(0), OUTPUT);
        touch(OUTPUT);
        const char* name = INPUT(1)->asCString();
        int index = OUTPUT->value_type->findFieldIndex(name);
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

    void setup(Branch& kernel)
    {
        SET_FIELD_FUNC = import_function(kernel, evaluate,
                "set_field(any, string, any) -> any");
        get_function_attrs(SET_FIELD_FUNC)->specializeType = specializeType;
        get_function_attrs(SET_FIELD_FUNC)->formatSource = formatSource;
    }
}
}
