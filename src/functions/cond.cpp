// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace cond_function {

    void cond_evaluate(caStack* stack)
    {
        int index = circa_bool_input(stack, 0) ? 1 : 2;
        copy(circa_input(stack, index), circa_output(stack, 0));
    }

    Type* specializeType(Term* caller)
    {
        List choices;
        set_type_list(&choices, caller->input(1)->type, caller->input(2)->type);
        return find_common_type(&choices);
    }

    void setup(Block* kernel)
    {
        FUNCS.cond = import_function(kernel, cond_evaluate,
                "cond(bool condition, any pos, any neg) -> any"
                "-- If 'condition' is true, returns 'pos'. Otherwise returns 'neg'.");
        function_set_specialize_type_func(FUNCS.cond, specializeType);
    }
}
}
