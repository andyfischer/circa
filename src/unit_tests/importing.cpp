// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "branch.h"
#include "importing.h"
#include "inspection.h"

namespace importing {

void my_native_patch(caStack*) {}

void bug_with_is_major_branch()
{
    Branch branch;
    Term* f = branch.compile("def f() {}");

    test_assert(is_major_branch(nested_contents(f)));
    
    // There was a bug where, if the function was patched with a native handler, it
    // would no longer be considered a major branch.

    install_function(f, my_native_patch);

    test_assert(is_major_branch(nested_contents(f)));
}

void register_tests()
{
    REGISTER_TEST_CASE(importing::bug_with_is_major_branch);
}

} // namespace importing
