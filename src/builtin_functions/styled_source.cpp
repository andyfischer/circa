// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include "circa.h"

namespace circa {
namespace styled_source_function {

    void format_source(EvalContext*, Term* caller)
    {
        Branch& branch = branch_ref_t::get_target_branch(caller->input(0));

        caller->reset();

        format_branch_source((StyledSource*) caller, branch);
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, format_source, "format_source(BranchRef b) -> StyledSource");
    }
}
}
