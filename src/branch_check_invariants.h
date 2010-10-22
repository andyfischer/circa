// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include "types/list.h"

namespace circa {

struct BranchInvariantCheck
{
    List errors;

    // Structure of each error:
    // [0] int type
    // [1] int index
    // [2] string message
};

void branch_check_invariants(BranchInvariantCheck* result, Branch& branch);
bool branch_check_invariants_print_result(Branch& branch, std::ostream& out);

} // namespace circa
