// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"
#include "refcounted_type_wrapper.h"

#include "branch_ref.h"

namespace circa {
namespace branch_ref_t {

    std::string toString(TaggedValue* tv)
    {
        char buf[40];
        sprintf(buf, "Branch@%p", tv->value_data.ptr);
        return buf;
    }

    void setup_type(Type* type)
    {
        reset_type(type);
        type->name = "BranchRef";
        type->toString = toString;
    }

} // namespace branch_ref_t


void BranchRef::set(Branch* branch)
{

}

Branch* BranchRef::get()
{
    return (Branch*) as_opaque_pointer(this);
}

} // namespace circa
