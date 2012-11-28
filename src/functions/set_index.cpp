// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace set_index_function {

    CA_FUNCTION(evaluate)
    {
        INCREMENT_STAT(SetIndex);

        copy(INPUT(0), OUTPUT);
        touch(OUTPUT);
        int index = INT_INPUT(1);
        copy(INPUT(2), list_get(OUTPUT,index));
    }

    Type* specializeType(Term* caller)
    {
        //FIXME
        return TYPES.list;
        //return caller->input(0)->type;
    }

    void setup(Block* kernel)
    {
        FUNCS.set_index = import_function(kernel, evaluate,
                "set_index(any list, int index, any val) -> List");
        function_set_specialize_type_func(FUNCS.set_index, specializeType);
    }
}
}
