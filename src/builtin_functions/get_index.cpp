// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace get_index_function {

    void evaluate(Term* caller)
    {
        Branch& input = caller->input(0)->asBranch();
        int index = caller->input(1)->asInt();

        if (index >= input.length()) {
            std::stringstream err;
            err << "Index " << index << " is out of range";
            error_occurred(caller, err.str());
            return;
        }
    
        assign_value(input[index], caller);
    }

    Term* specializeType(Term* caller)
    {
        Branch& input = caller->input(0)->asBranch();

        if (input.length() > 0)
            return input[0]->type;
        else
            return ANY_TYPE;
    }

    void setup(Branch& kernel)
    {
        GET_INDEX_FUNC = import_function(kernel, evaluate, "get_index(any, int) : any");
        as_function(GET_INDEX_FUNC).specializeType = specializeType;
    }
}
}
