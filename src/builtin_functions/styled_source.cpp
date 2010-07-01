// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include "circa.h"

namespace circa {
namespace styled_source_function {

    CA_FUNCTION(format_source)
    {
        Branch& branch = branch_ref_t::get_target_branch(INPUT(0));

        OUTPUT->reset();
        format_branch_source((StyledSource*) OUTPUT, branch);
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, format_source, "format_source(BranchRef b) -> StyledSource");
    }
}
}
