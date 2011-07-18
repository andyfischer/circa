// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "importing.h"

namespace circa {
namespace reflect_function {

    CA_FUNCTION(this_branch)
    {
        set_branch(OUTPUT, CALLER->owningBranch);
    }
    void setup(Branch& kernel)
    {
        Branch& ns = create_namespace(kernel, "reflect");
        import_function(ns, this_branch, "this_branch() -> Branch");
    }
}
} // namespace circa
