// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace get_field_function {

    void evaluate(EvalContext* cxt, Term* caller)
    {
        Term* head = caller->input(0);

        for (int nameIndex=1; nameIndex < caller->numInputs(); nameIndex++) {
            std::string const& name = caller->input(nameIndex)->asString();

            int fieldIndex = head->value_type->findFieldIndex(name);

            if (fieldIndex == -1) {
                // assert(false);
                error_occurred(cxt, caller, "field not found: " + name);
                return;
            }

            head = as_branch(head)[fieldIndex];
        }

        copy(head, caller);
    }

    Term* specializeType(Term* caller)
    {
        Term* head = caller->input(0);

        for (int nameIndex=1; nameIndex < caller->numInputs(); nameIndex++) {
            std::string const& name = caller->input(1)->asString();

            Branch& type_prototype = type_t::get_prototype(head->type);

            if (!type_prototype.contains(name))
                return ANY_TYPE;

            head = type_prototype[name];
        }

        return head->type;
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
        function_t::get_attrs(GET_FIELD_FUNC).specializeType = specializeType;
        function_t::get_attrs(GET_FIELD_FUNC).formatSource = formatSource;
    }
}
}
