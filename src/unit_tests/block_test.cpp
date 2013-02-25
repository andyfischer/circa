// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "block.h"
#include "stateful_code.h"
#include "type.h"

namespace block_test {

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

    //test_equals(&block.stateType->parameter, "[[<Type int>, <Type number>], ['a', 'b']]");
    test_equals(&block.stateType->parameter, "[[<Type any>, <Type any>], ['a', 'b']]");
}

void test_erase_term()
{
    Block block;
    block.compile("a = 1");
    block.compile("b = 2");
    block.compile("c = 3");
    
    test_equals(block.get(0)->nameStr(), "a");
    test_equals(block.get(1)->nameStr(), "b");
    test_equals(block.get(2)->nameStr(), "c");

    erase_term(block.get(1));

    test_equals(block.get(0)->nameStr(), "a");
    test_assert(block.get(1) == NULL);
    test_equals(block.get(2)->nameStr(), "c");

    remove_nulls(&block);

    test_equals(block.get(0)->nameStr(), "a");
    test_equals(block.get(1)->nameStr(), "c");
}

void register_tests()
{
    REGISTER_TEST_CASE(block_test::test_state_type);
    REGISTER_TEST_CASE(block_test::test_erase_term);
}

}
