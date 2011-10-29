
// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include <circa.h>
#include <importing_macros.h>

namespace circa {
namespace subroutine_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(subroutine_output,
        "subroutine_output(any :optional :multiple)")
    {
        copy(INPUT(0), &CONTEXT->subroutineOutput);
#if 0
        out->resize(NUM_INPUTS);
        for (int i=0; i < NUM_INPUTS; i++) {
            if (INPUT_TERM(i) == NULL)
                set_null(out->get(i));
            else
                consume_input(CONTEXT, CALLER, i, out->get(i));
        }
#endif
    }

    void setup(Branch* kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
        SUBROUTINE_OUTPUT_FUNC = kernel->get("subroutine_output");
    }
} // namespace subroutine_function
} // namespace circa
