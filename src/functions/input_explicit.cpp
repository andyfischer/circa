// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace input_explicit_function {

    void input_explicit(caStack* stack)
    {
        Term* inputTerm = (Term*) circa_term_get_input(circa_caller_term(stack), 0);
        caValue* input = find_stack_value_for_term(stack, inputTerm, 0);

        copy(input, circa_output(stack, 0));
    }

    void input_postCompile(Term* term)
    {
        Block* block = term->owningBlock;

        Term* in = append_input_placeholder(block);
        set_input(term, 0, in);
        term->inputInfo(0)->properties.setBool("hidden", true);
    }

    void output_postCompile(Term* term)
    {
        Term* out = append_output_placeholder(term->owningBlock, term->input(0));
        hide_from_source(out);
    }

    void setup(Block* kernel)
    {
        FUNCS.input_explicit = import_function(kernel, input_explicit, "input(any _) -> any");
        as_function(FUNCS.input_explicit)->postCompile = input_postCompile;
        
        FUNCS.output_explicit = import_function(kernel, NULL, "output(any _)");
        as_function(FUNCS.output_explicit)->postCompile = output_postCompile;
    }
}
}
