// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace term_to_source_function {

    void evaluate(caStack* stack)
    {
        Term* term = circa_caller_input_term(stack, 0);
        set_string(circa_output(stack, 0), get_term_source_text(term));
    }

    void setup(Block* kernel)
    {
        import_function(kernel, evaluate, "term_to_source(any term :meta) -> String");
    }
}
} // namespace circa
