// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include "circa.h"

namespace circa {
namespace neg_function {

    void evaluate_f(Term* caller)
    {
        as_float(caller) = -float_input(caller,0);
    }

    void evaluate_i(Term* caller)
    {
        as_int(caller) = -int_input(caller,0);
    }

    std::string toSourceString(Term* term)
    {
        return "-" + get_source_of_input(term, 0);
    }

    void setup(Branch& kernel)
    {
        Term* neg_i = import_function(kernel, evaluate_i, "neg_i(int) -> int");
        Term* neg_f = import_function(kernel, evaluate_f, "neg_f(number) -> number");

        function_t::get_to_source_string(neg_i) = toSourceString;
        function_t::get_to_source_string(neg_f) = toSourceString;

        NEG_FUNC = create_overloaded_function(kernel, "neg");
        create_ref(as_branch(NEG_FUNC), neg_i);
        create_ref(as_branch(NEG_FUNC), neg_f);
    }
}
}
