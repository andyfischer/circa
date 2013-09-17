// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "inspection.h"
#include "loops.h"

namespace loop_test {

void test_list_names_that_must_be_looped()
{
    Value names;

    Block block;
    block.compile("section { x = 1 }");
    list_names_that_must_be_looped(find_block_from_path(&block, "function=section"), &names);
    test_equals(&names, "[]");

    clear_block(&block);
    block.compile("x = 1; section { x = 2 }");
    list_names_that_must_be_looped(find_block_from_path(&block, "function=section"), &names);
    test_equals(&names, "[]");

    clear_block(&block);
    block.compile("section { x = 0; x = x + 1 }");
    list_names_that_must_be_looped(find_block_from_path(&block, "function=section"), &names);
    test_equals(&names, "[]");

    clear_block(&block);
    block.compile("x = 0; section { x = 0; x = x + 1 }");
    list_names_that_must_be_looped(find_block_from_path(&block, "function=section"), &names);
    test_equals(&names, "[]");

    clear_block(&block);
    block.compile("x = 0; section { x = x + 1 }");
    list_names_that_must_be_looped(find_block_from_path(&block, "function=section"), &names);
    test_equals(&names, "['x']");

    clear_block(&block);
    block.compile("x = 0; section { y = x + 1 }");
    list_names_that_must_be_looped(find_block_from_path(&block, "function=section"), &names);
    test_equals(&names, "[]");

    clear_block(&block);
    block.compile("x = 0; y = 1; z = 2; section { x += 1; y += 1; z += 1 }");
    list_names_that_must_be_looped(find_block_from_path(&block, "function=section"), &names);
    test_equals(&names, "['x', 'y', 'z']");
}

void register_tests()
{
    REGISTER_TEST_CASE(loop_test::test_list_names_that_must_be_looped);
}

} // namespace loop_test
