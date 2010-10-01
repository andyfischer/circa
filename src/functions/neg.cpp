// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include "circa.h"

namespace circa {
namespace neg_function  {

    CA_FUNCTION(evaluate_f)
    {
        make_float(OUTPUT, -FLOAT_INPUT(0));
    }

    CA_FUNCTION(evaluate_i)
    {
        make_int(OUTPUT, -INT_INPUT(0));
    }

    void formatSource(StyledSource* source, Term* term)
    {
        append_phrase(source, "-", term, phrase_type::INFIX_OPERATOR);
        format_source_for_input(source, term, 0);
    }

    void setup(Branch& kernel)
    {
        Term* neg_i = import_function(kernel, evaluate_i, "neg_i(int) -> int");
        Term* neg_f = import_function(kernel, evaluate_f, "neg_f(number) -> number");

        function_t::get_attrs(neg_i).formatSource = formatSource;
        function_t::get_attrs(neg_f).formatSource = formatSource;

        NEG_FUNC = create_overloaded_function(kernel, "neg", RefList(neg_i, neg_f));
        function_t::get_attrs(NEG_FUNC).formatSource = formatSource;
    }
}
}
