// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "build_options.h"
#include "branch.h"
#include "introspection.h"

#include "debug.h"

namespace circa {

bool DEBUG_TRAP_NAME_LOOKUP = false;
bool DEBUG_TRAP_ERROR_OCCURRED = false;

void dump(Branch& branch)
{
    print_branch(std::cout, branch);
}

void dump_with_props(Branch& branch)
{
    print_branch_with_properties(std::cout, branch);
}

void dump(Value& value)
{
    std::cout << value.toString() << std::endl;
}
void dump(Value* value)
{
    std::cout << value->toString() << std::endl;
}

} // namespace circa
