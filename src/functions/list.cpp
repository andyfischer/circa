// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace list_function {

    CA_START_FUNCTIONS;

    Term* specializeType(Term* term)
    {
        RefList inputTypes;
        for (int i=0; i < term->numInputs(); i++)
            inputTypes.append(term->input(i)->type);

        return create_implicit_tuple_type(inputTypes);
    }

    CA_DEFINE_FUNCTION(evaluate, "list(any...) -> List")
    {
        make_list(OUTPUT);
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

    CA_DEFINE_FUNCTION(repeat, "repeat(any, int) -> List")
    {
        make_list(OUTPUT);
        List* result = List::checkCast(OUTPUT);
        TaggedValue* source = INPUT(0);
        int repeatCount = INT_INPUT(1);

        result->resize(repeatCount);

        for (int i=0; i < repeatCount; i++)
            copy(source, result->get(i));
    }

    CA_DEFINE_FUNCTION(blank_list, "blank_list(int) -> List")
    {
        make_list(OUTPUT);
        List* result = List::checkCast(OUTPUT);
        result->resize(0);
        result->resize(INT_INPUT(0));
    }

    CA_DEFINE_FUNCTION(resize, "resize(List, int) -> List")
    {
        copy(INPUT(0), OUTPUT);
        List* result = List::checkCast(OUTPUT);
        result->resize(INT_INPUT(1));
    }

    void setup(Branch& kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);

        LIST_FUNC = kernel["list"];

        function_t::get_attrs(LIST_FUNC).specializeType = specializeType;
        function_t::get_attrs(LIST_FUNC).formatSource = list_formatSource;
    }
}
}
