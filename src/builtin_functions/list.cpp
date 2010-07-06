// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace list_function {

    Term* specializeType(Term* term)
    {
        RefList inputTypes;
        for (int i=0; i < term->numInputs(); i++)
            inputTypes.append(term->input(i)->type);

        return create_implicit_tuple_type(inputTypes);
    }

    CA_FUNCTION(evaluate)
    {
        List* result = List::checkCast(OUTPUT);

        result->resize(NUM_INPUTS);

        for (int i=0; i < NUM_INPUTS; i++)
            copy(INPUT(i), (*result)[i]);
    }

    void list_formatSource(StyledSource* source, Term* caller)
    {
        format_name_binding(source, caller);
        append_phrase(source, "[", caller, token::LBRACKET);
        for (int i=0; i < caller->numInputs(); i++)
            format_source_for_input(source, caller, i);
        append_phrase(source, "]", caller, token::LBRACKET);
    }

    CA_FUNCTION(evaluate_repeat)
    {
        List* result = List::checkCast(OUTPUT);
        TaggedValue* source = INPUT(0);
        int repeatCount = INT_INPUT(1);

        result->resize(repeatCount);

        for (int i=0; i < repeatCount; i++)
            copy(source, result->get(i));
    }

    void setup(Branch& kernel)
    {
        LIST_FUNC = import_function(kernel, evaluate, "list(any...) -> List");
        function_t::get_attrs(LIST_FUNC).specializeType = specializeType;
        function_t::get_attrs(LIST_FUNC).formatSource = list_formatSource;

        import_function(kernel, evaluate_repeat, "repeat(any, int) -> List");
    }
}
}
