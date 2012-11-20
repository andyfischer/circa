// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "block.h"
#include "type.h"

namespace block {

void test_state_type()
{
    Block block;

    block.compile("a = 1; b = 2; c = 3");
    block_update_state_type(&block);
    test_assert(block.stateType == NULL);

    clear_block(&block);
    block.compile("state a = 1; state b = 2.0");
    block_update_state_type(&block);
    test_assert(block.stateType != NULL);

    test_equals(&block.stateType->parameter, "[[<Type any>, <Type any>], ['a', 'b']]");
}

void register_tests()
{
    REGISTER_TEST_CASE(block::test_state_type);
}

}
