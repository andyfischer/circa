// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

namespace if_block_test {

void dont_rebind_nonlocal_name()
{
    Block block;
    block.compile("if true { sum = 3 }");
    test_assert(find_local_name(&block, "sum") == NULL);
}

void register_tests()
{
    REGISTER_TEST_CASE(if_block_test::dont_rebind_nonlocal_name);
}

} // namespace if_block_test
