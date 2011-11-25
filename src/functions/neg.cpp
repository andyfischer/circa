// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include "circa.h"

namespace circa {
namespace neg_function  {

    CA_FUNCTION(evaluate_f)
    {
        set_float(OUTPUT, -FLOAT_INPUT(0));
    }

    CA_FUNCTION(evaluate_i)
    {
        set_int(OUTPUT, -INT_INPUT(0));
    }

    void formatSource(StyledSource* source, Term* term)
    {
        append_phrase(source, "-", term, phrase_type::INFIX_OPERATOR);
        format_source_for_input(source, term, 0);
    }

    void setup(Branch* kernel)
    {
        Term* neg_i = import_function(kernel, evaluate_i, "neg_i(int) -> int");
        Term* neg_f = import_function(kernel, evaluate_f, "neg_f(number) -> number");

        as_function(neg_i)->formatSource = formatSource;
        as_function(neg_f)->formatSource = formatSource;

        TermList overloads(neg_i, neg_f);
        NEG_FUNC = create_overloaded_function(kernel, "neg", &overloads);
        as_function(NEG_FUNC)->formatSource = formatSource;
    }
}
}
