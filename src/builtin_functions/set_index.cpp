// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace set_index_function {

    void evaluate(Term* caller)
    {
        assign_value(caller->input(0), caller);

        int index = caller->input(1)->asInt();
        assign_value(caller->input(2), as_branch(caller)[index]);
    }

    Term* specializeType(Term* caller)
    {
        return caller->input(0)->type;
    }

    void setup(Branch& kernel)
    {
        SET_INDEX_FUNC = import_function(kernel, evaluate,
                "set_index(any, int, any) : any");
        as_function(SET_INDEX_FUNC).specializeType = specializeType;
    }
}
}
