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

void dump(TaggedValue& value)
{
    std::cout << value.toString() << std::endl;
}
void dump(TaggedValue* value)
{
    std::cout << value->toString() << std::endl;
}

#if CIRCA_ENABLE_TRAP_ON_VALUE_WRITE

void debug_trap_value_write(TaggedValue* val)
{
    ;
}

#endif

} // namespace circa
