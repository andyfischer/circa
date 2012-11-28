// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace unknown_function_function {

    void evaluate_unknown_function(caStack* stack)
    {
        std::string out;
        out += "Unknown function: ";
        Term* caller = (Term*) circa_caller_term(stack);
        out += caller->stringProp("syntax:functionName", "");
        circa_output_error(stack, out.c_str());
    }

    void setup(Block* kernel)
    {
        FUNCS.unknown_function = import_function(kernel, evaluate_unknown_function,
            "unknown_function(any ins :multiple) -> any");
    }
}
}
