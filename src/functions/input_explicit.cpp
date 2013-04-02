// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace input_explicit_function {

    void input_explicit(caStack* stack)
    {
        copy(circa_input(stack, 1), circa_output(stack, 0));
    }

    void input_postCompile(Term* term)
    {
        Block* block = term->owningBlock;

        Term* in = append_input_placeholder(block);
        set_input(term, 1, in);
        term->inputInfo(1)->properties.setBool("hidden", true);

        if (term->input(0) != NULL) {
            Type* type = as_type(term_value(term->input(0)));
            change_declared_type(in, type);
            change_declared_type(term, type);
        }

    }

    void output_postCompile(Term* term)
    {
        Term* out = insert_output_placeholder(term->owningBlock, term->input(0), 0);
        hide_from_source(out);
    }

    void setup(Block* kernel)
    {
        FUNCS.input_explicit = import_function(kernel, input_explicit,
                "input(Type t :optional, any _) -> any");
        as_function(FUNCS.input_explicit)->postCompile = input_postCompile;
        
        FUNCS.output_explicit = import_function(kernel, NULL, "output(any _)");
        as_function(FUNCS.output_explicit)->postCompile = output_postCompile;
    }
}
}
