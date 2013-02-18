// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace get_field_function {

    void evaluate(caStack* stack)
    {
        caValue* head = circa_input(stack, 0);
        const char* keyStr = circa_string_input(stack, 1);

        Value error;
        caValue* value = get_field2(head, circa_input(stack, 1), &error);

        if (!is_null(&error)) {
            circa_output_error(stack, as_cstring(&error));
            return;
        }

        ca_assert(value != NULL);

        copy(value, circa_output(stack, 0));
    }

    Type* specializeType(Term* caller)
    {
        Type* head = caller->input(0)->type;

        for (int nameIndex=1; nameIndex < caller->numInputs(); nameIndex++) {

            // Abort if input type is not correct
            if (!is_string(term_value(caller->input(1))))
                return TYPES.any;

            if (!is_list_based_type(head))
                return TYPES.any;

            std::string const& name = as_string(term_value(caller->input(1)));

            int fieldIndex = list_find_field_index_by_name(head, name.c_str());

            if (fieldIndex == -1)
                return TYPES.any;

            head = as_type(get_index(list_get_type_list_from_type(head),fieldIndex));
        }

        return head;
    }

    void formatSource(caValue* source, Term* term)
    {
        format_name_binding(source, term);
        //append_phrase(source, get_relative_name(term, term->input(0)),
        //        term, sym_None);
        format_source_for_input(source, term, 0);
        for (int i=1; i < term->numInputs(); i++) {
            append_phrase(source, ".", term, tok_Dot);
            append_phrase(source, as_string(term_value(term->input(i))),
                    term, tok_Identifier);
        }
    }

    void setup(Block* kernel)
    {
        FUNCS.get_field = import_function(kernel, evaluate,
                "get_field(any obj, String key) -> any");
        as_function(FUNCS.get_field)->specializeType = specializeType;
        as_function(FUNCS.get_field)->formatSource = formatSource;
    }
}
}
