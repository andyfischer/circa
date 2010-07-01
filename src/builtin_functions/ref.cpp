// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

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
