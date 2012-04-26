
// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace input_explicit_function {

    void input_explicit(caStack* stack)
    {
        copy(circa_input(stack, 0), circa_output(stack, 0));
    }

    void postCompile(Term* term)
    {
        Branch* branch = term->owningBranch;

        Term* in = append_input_placeholder(branch);
        set_input(term, 0, in);
        term->inputInfo(0)->properties.setBool("hidden", true);
    }

    void setup(Branch* kernel)
    {
        FUNCS.input_explicit = import_function(kernel, input_explicit, "input() -> any");
        as_function(FUNCS.input_explicit)->postCompile = postCompile;
        
        FUNCS.output_explicit = import_function(kernel, NULL, "output(any)");
    }
}
}
