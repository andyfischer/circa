// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "block.h"
#include "importing.h"
#include "inspection.h"

namespace importing_test {

void my_native_patch(caStack*) {}

void bug_with_is_major_block()
{
    Block block;
    Term* f = block.compile("def f() {}");

    test_assert(is_major_block(nested_contents(f)));
    
    // There was a bug where, if the function was patched with a native handler, it
    // would no longer be considered a major block.

    install_function(f, my_native_patch);

    test_assert(is_major_block(nested_contents(f)));
}

void register_tests()
{
    REGISTER_TEST_CASE(importing_test::bug_with_is_major_block);
}

} // namespace importing_test
