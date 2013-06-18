// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace to_string_function {

    void evaluate(caStack* stack)
    {
        caValue* in = circa_input(stack, 0);
        caValue* out = circa_output(stack, 0);
        if (is_string(in))
            copy(in, out);
        else
            set_string(out, to_string(in));
    }

    void setup(Block* kernel)
    {
        import_function(kernel, evaluate, "to_string(any val) -> String");
    }
}
} // namespace circa
