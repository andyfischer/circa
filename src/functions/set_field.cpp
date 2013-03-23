// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace set_field_function {

    void set_field(caStack* stack)
    {
        INCREMENT_STAT(SetField);

        caValue* out = circa_output(stack, 0);
        copy(circa_input(stack, 0), out);
        touch(out);

        caValue* name = circa_input(stack, 1);

        caValue* slot = get_field(out, name, NULL);
        if (slot == NULL) {
            circa::Value msg;
            set_string(&msg, "field not found: ");
            string_append(&msg, name);
            return raise_error_msg(stack, as_cstring(&msg));
        }

        copy(circa_input(stack, 2), slot);
    }

    Type* specializeType(Term* caller)
    {
        return caller->input(0)->type;
    }

    void formatSource(caValue* source, Term* term)
    {
        format_source_for_input(source, term, 0);
        for (int i=2; i < term->numInputs(); i++) {
            append_phrase(source, ".", term, sym_None);
            caValue* fieldName = term_value(term->input(i));

            if (is_string(fieldName))
                append_phrase(source, as_cstring(fieldName), term, sym_None);
            else
                // fieldName isn't a string, this is unexpected
                append_phrase(source, to_string(fieldName).c_str(), term, sym_None);
        }
        append_phrase(source, " =", term, sym_None);
        format_source_for_input(source, term, 1);
    }

    void setup(Block* kernel)
    {
        FUNCS.set_field = import_function(kernel, set_field,
                "set_field(any obj, String key, any val) -> any");
        as_function(FUNCS.set_field)->specializeType = specializeType;
        as_function(FUNCS.set_field)->formatSource = formatSource;
    }
}
}
