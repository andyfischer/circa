// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include "circa.h"

namespace circa {

bool DEBUG_TRAP_NAME_LOOKUP = false;
bool DEBUG_TRAP_ERROR_OCCURRED = false;

void dump_branch(Branch& branch)
{
    print_branch_raw(std::cout, branch);
}

void dump_bytecode(Branch& branch)
{
    bytecode::print_bytecode_for_all_major_branches(std::cout, branch);
}

void dump_branch_with_props(Branch& branch)
{
    print_branch_raw_with_properties(std::cout, branch);
}

} // namespace circa
