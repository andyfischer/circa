// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace ref_function {

    void ref(caStack* stack)
    {
        caTerm* term = circa_caller_input_term(stack, 0);
        set_ref(circa_output(stack, 0), (Term*) term);
    }

    void setup(Branch* kernel)
    {
        REF_FUNC = import_function(kernel, ref, "ref(any :ignore_error) -> Term");
    }
}
} // namespace circa
