// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"
#include "importing_macros.h"

namespace circa {
namespace ref_function {

    CA_FUNCTION(ref)
    {
        as_ref(OUTPUT) = INPUT_TERM(0);
    }

    void setup(Branch& kernel)
    {
        REF_FUNC = import_function(kernel, ref, "ref(any) -> Ref");
    }
}
} // namespace circa
