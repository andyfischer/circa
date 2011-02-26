// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

namespace circa {
namespace branch_ref_t {

    void setup_type(Type* type);

} // namespace branch_ref_t

struct BranchRef : TaggedValue
{
    void set(Branch*);
    Branch* get();
};

} // namespace circa
