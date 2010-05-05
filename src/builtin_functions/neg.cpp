// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include "circa.h"

namespace circa {
namespace neg_function {

    void evaluate_f(EvalContext*, Term* caller)
    {
        set_float(caller, -float_input(caller,0));
    }

    void evaluate_i(EvalContext*, Term* caller)
    {
        set_int(caller, -int_input(caller,0));
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
    }
}
}
