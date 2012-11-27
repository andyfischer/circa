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

    void postCompile(Term* term)
    {
        Block* block = term->owningBlock;

        Term* in = append_input_placeholder(block);
        set_input(term, 0, in);
        term->inputInfo(0)->properties.setBool("hidden", true);
    }

    void setup(Block* kernel)
    {
        FUNCS.input_explicit = import_function(kernel, input_explicit, "input(any _) -> any");
        as_function(FUNCS.input_explicit)->postCompile = postCompile;
        
        FUNCS.output_explicit = import_function(kernel, NULL, "output(any _)");
    }
}
}
