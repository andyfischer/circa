// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"

namespace circa {
namespace branch_ref_function {

    CA_FUNCTION(branch_ref)
    {
        List& output = *List::cast(OUTPUT, 1);
        Branch* ptr = &(INPUT_TERM(0)->nestedContents);
        set_opaque_pointer(output[0], ptr);
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, branch_ref,
            "def branch_ref(any branch +ignore_error) -> BranchRef");
    }
}
}
