// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include <circa.h>
#include "builtins.h"

namespace circa {
namespace meta_function {

    void lift_closure(Branch* branch)
    {
        for (int i=0; i < branch->length(); i++) {
            Term* term = branch->get(i);
            if (term == NULL) continue;
            if (term->function == FREEZE_FUNC) {
                Term* input = term->input(0);

                EvalContext context;
                evaluate_minimum(&context, input, NULL);

                change_function(term, VALUE_FUNC);
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
        //FIXME lift_closure(as_branch(OUTPUT));
    }

    void setup(Branch* kernel)
    {
        #if 0
        FIXME
        FREEZE_FUNC = import_function(kernel, copy_function::evaluate, "freeze(any) -> any");
        function_t::get_specialize_type(FREEZE_FUNC) = freeze_specializeType;

        import_function(kernel, lift_closure_evaluate, "lift_closure(Branch) -> Branch");
        #endif
    }

} // namespace meta_function
} // namespace circa
