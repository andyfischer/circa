// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace neg_function  {

    void evaluate_f(caStack* stack)
    {
        set_float(circa_output(stack, 0), -circa_float_input(stack, 0));
    }

    void evaluate_i(caStack* stack)
    {
        set_int(circa_output(stack, 0), -circa_int_input(stack, 0));
    }

    void formatSource(caValue* source, Term* term)
    {
        append_phrase(source, "-", term, sym_InfixOperator);
        format_source_for_input(source, term, 0);
    }

    void setup(Block* kernel)
    {
        Term* neg_i = import_function(kernel, evaluate_i, "neg_i(int i) -> int");
        Term* neg_f = import_function(kernel, evaluate_f, "neg_f(number n) -> number");

        block_set_format_source_func(function_contents(neg_i), formatSource);
        block_set_format_source_func(function_contents(neg_f), formatSource);
    }
}
}
