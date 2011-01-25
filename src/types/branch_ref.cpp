// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"
#include "refcounted_type_wrapper.h"

namespace circa {
namespace branch_ref_t {

    void initialize(Branch& kernel)
    {
        Type* type = Type::create();
        type->name = "BranchRef";
        intrusive_refcounted::setup_type<Branch>(type);

        Term* term = create_type(kernel, "BranchRef");
        set_type(term, type);
    }

} // namespace branch_ref_t
} // namespace circa
