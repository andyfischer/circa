// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include "circa.h"

namespace circa {
namespace lookup_branch_ref_function {

    void evaluate(EvalContext*, Term* caller)
    {
        std::string name = as_string(caller->input(0));
        Term* term = get_global(name);

        if (term == NULL)
            return branch_ref_t::set_from_ref(caller, NULL);

        if (!is_branch(term))
            return branch_ref_t::set_from_ref(caller, NULL);

        return branch_ref_t::set_from_ref(caller, term);
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "lookup_branch_ref(string) -> BranchRef");
    }
}
}
