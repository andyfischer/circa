// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace get_field_function {

    CA_FUNCTION(evaluate)
    {
        caValue* head = circa_input(STACK, 0);
        const char* keyStr = circa_string_input(STACK, 1);
        
        if (!is_list_based_type(head->value_type)) {
            std::string msg = "get_field failed, not a compound type: " + to_string(head);
            msg += ". Field is: ";
            msg += keyStr;
            return RAISE_ERROR(msg.c_str());
        }

        int fieldIndex = list_find_field_index_by_name(head->value_type, keyStr);

        if (fieldIndex == -1) {
            std::string msg = "field not found: ";
            msg += keyStr;
            return RAISE_ERROR(msg.c_str());
        }

        copy(circa_index(head, fieldIndex), circa_output(STACK, 0));
    }

    Type* specializeType(Term* caller)
    {
        Type* head = caller->input(0)->type;

        for (int nameIndex=1; nameIndex < caller->numInputs(); nameIndex++) {

            // Abort if input type is not correct
            if (!is_string(term_value(caller->input(1))))
                return &ANY_T;

            if (!is_list_based_type(head))
                return &ANY_T;

            std::string const& name = as_string(term_value(caller->input(1)));

            int fieldIndex = list_find_field_index_by_name(head, name.c_str());

            if (fieldIndex == -1)
                return &ANY_T;

            head = as_type(get_index(list_get_type_list_from_type(head),fieldIndex));
        }

        return head;
    }

    void formatSource(caValue* source, Term* term)
    {
        format_name_binding(source, term);
        //append_phrase(source, get_relative_name(term, term->input(0)),
        //        term, name_None);
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
                "get_field(any, String) -> any");
        as_function(FUNCS.get_field)->specializeType = specializeType;
        as_function(FUNCS.get_field)->formatSource = formatSource;
    }
}
}
