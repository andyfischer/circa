// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace ref_function {

    CA_FUNCTION(ref)
    {
        set_ref(OUTPUT, INPUT_TERM(0));
    }

    void setup(Branch* kernel)
    {
        REF_FUNC = import_function(kernel, ref, "ref(any :ignore_error) -> Term");
    }
}
} // namespace circa
