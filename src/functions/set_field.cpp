// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace set_field_function {

    CA_FUNCTION(evaluate)
    {
        INCREMENT_STAT(setField);

        copy(INPUT(0), OUTPUT);
        touch(OUTPUT);

        const char* name = as_cstring(INPUT(1));
        int index = list_find_field_index_by_name(OUTPUT->value_type, name);
        if (index == -1) {
            std::string msg = std::string("field not found: ") + name;
            return RAISE_ERROR(msg.c_str());
        }
        copy(INPUT(2), list_get(OUTPUT,index));
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
            caValue* fieldName = term_value(term->input(i));

            if (is_string(fieldName))
                append_phrase(source, as_cstring(fieldName), term, phrase_type::UNDEFINED);
            else
                // fieldName isn't a string, this is unexpected
                append_phrase(source, to_string(fieldName).c_str(), term, phrase_type::UNDEFINED);
        }
        append_phrase(source, " =", term, phrase_type::UNDEFINED);
        format_source_for_input(source, term, 1);
    }

    void setup(Branch* kernel)
    {
        FUNCS.set_field = import_function(kernel, evaluate,
                "set_field(any, String, any) -> any");
        as_function(FUNCS.set_field)->specializeType = specializeType;
        as_function(FUNCS.set_field)->formatSource = formatSource;
    }
}
}
