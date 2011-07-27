// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"

namespace circa {
namespace get_field_function {

    CA_FUNCTION(evaluate)
    {
        TaggedValue* head = INPUT(0);

        for (int nameIndex=1; nameIndex < NUM_INPUTS; nameIndex++) {
            std::string const& name = INPUT(nameIndex)->asString();

            int fieldIndex = head->value_type->findFieldIndex(name);

            if (fieldIndex == -1)
                return error_occurred(CONTEXT, CALLER, "field not found: " + name);

            head = head->getIndex(fieldIndex);
        }

        copy(head, OUTPUT);
    }

    Term* specializeType(Term* caller)
    {
        Term* head = caller->input(0);

        for (int nameIndex=1; nameIndex < caller->numInputs(); nameIndex++) {

            // Abort if input type is not correct
            if (!is_string(caller->input(1)))
                return ANY_TYPE;

            std::string const& name = caller->input(1)->asString();

            Branch& type_prototype = type_t::get_prototype(declared_type(head));

            if (!type_prototype.contains(name))
                return ANY_TYPE;

            head = type_prototype[name];
        }

        return head->type;

#if 0

WIP code

        Type* headType = declared_type(caller->input(0));

        for (int nameIndex=1; nameIndex < caller->numInputs(); nameIndex++) {

            // Abort if input type is not correct
            if (!is_string(caller->input(1)))
                return ANY_TYPE;

            std::string const& name = caller->input(1)->asString();

            if (!is_list_based_type(declared_type(head)))
                return ANY_TYPE;

            TaggedValue* nameList = list_get_name_list_from_parameter(declared_type(head));

            if (nameList == NULL)
                return ANY_TYPE;

            List& names = *as_list(nameList);

            for (int i=0; i < names.length(); i++) {
                if (names[i] == name) {
                }
            }

            

            Branch& type_prototype = type_t::get_prototype(declared_type(head));

            if (!type_prototype.contains(name))
                return ANY_TYPE;

            head = type_prototype[name];
        }

        return head->type;
#endif
    }

    void formatSource(StyledSource* source, Term* term)
    {
        format_name_binding(source, term);
        //append_phrase(source, get_relative_name(term, term->input(0)),
        //        term, phrase_type::UNDEFINED);
        format_source_for_input(source, term, 0);
        for (int i=1; i < term->numInputs(); i++) {
            append_phrase(source, ".", term, token::DOT);
            append_phrase(source, term->input(i)->asString(),
                    term, token::IDENTIFIER);
        }
    }

    void setup(Branch& kernel)
    {
        GET_FIELD_FUNC = import_function(kernel, evaluate,
                "get_field(any, string...) -> any");
        get_function_attrs(GET_FIELD_FUNC)->specializeType = specializeType;
        get_function_attrs(GET_FIELD_FUNC)->formatSource = formatSource;
    }
}
}
