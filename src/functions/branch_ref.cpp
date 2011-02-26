// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"

namespace circa {
namespace branch_ref_function {

    #if 0
    void set_from_ref(TaggedValue* value, Term* ref)
    {
        List* list = List::checkCast(value);
        touch(list);
        set_ref(list->getIndex(0), ref);
    }
    #endif

    CA_FUNCTION(branch_ref)
    {
        //branch_ref_t::set_from_ref(OUTPUT, INPUT_TERM(0));
    }

    void setup(Branch& kernel)
    {
        //import_function(kernel, branch_ref,
            //"def branch_ref(any branch +ignore_error) -> BranchRef");
    }
}
}
