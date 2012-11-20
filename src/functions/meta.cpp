// Copyright (c) 2007-2010 Andrew Fischer. All rights reserved

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace meta_function {

    void lift_closure(Block* block)
    {
        for (int i=0; i < block->length(); i++) {
            Term* term = block->get(i);
            if (term == NULL) continue;
            if (term->function == FUNCS.freeze) {
                Term* input = term->input(0);

                Stack context;
                evaluate_minimum(&context, input, NULL);

                change_function(term, FUNCS.value);
            }
        }
    }

    Type* freeze_specializeType(Term* term)
    {
        return term->input(0)->type;
    }

    CA_FUNCTION(lift_closure_evaluate)
    {
        copy(INPUT(0), OUTPUT);
        touch(OUTPUT);
        //FIXME lift_closure(as_block(OUTPUT));
    }

    void setup(Block* kernel)
    {
        #if 0
        FIXME
        FUNCS.freeze = import_function(kernel, copy_function::evaluate, "freeze(any) -> any");
        function_t::get_specialize_type(FUNCS.freeze) = freeze_specializeType;

        import_function(kernel, lift_closure_evaluate, "lift_closure(Block) -> Block");
        #endif
    }

} // namespace meta_function
} // namespace circa
