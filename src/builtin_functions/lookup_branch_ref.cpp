// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include "circa.h"

namespace circa {
namespace lookup_branch_ref_function {

    CA_FUNCTION(evaluate)
    {
        std::string name = as_string(INPUT(0));
        Term* term = get_global(name);

        if (term == NULL)
            return branch_ref_t::set_from_ref(OUTPUT, NULL);

        if (!is_branch(term))
            return branch_ref_t::set_from_ref(OUTPUT, NULL);

        return branch_ref_t::set_from_ref(OUTPUT, term);
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "lookup_branch_ref(string) -> BranchRef");
    }
}
}
