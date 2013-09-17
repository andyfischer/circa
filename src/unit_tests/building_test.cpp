// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "block.h"
#include "building.h"
#include "inspection.h"
#include "type.h"

namespace building_test {

void test_insert_output_placeholder()
{
    Block block;

    insert_output_placeholder(&block, NULL, 0)->setStringProp("note", "first");

    test_equals(count_output_placeholders(&block), 1);
    test_equals(get_output_placeholder(&block, 0)->getProp("note"), "first");

    insert_output_placeholder(&block, NULL, 0)->setStringProp("note", "second");
    test_equals(count_output_placeholders(&block), 2);
    test_equals(get_output_placeholder(&block, 0)->getProp("note"), "second");
    test_equals(get_output_placeholder(&block, 1)->getProp("note"), "first");

    insert_output_placeholder(&block, NULL, 1)->setStringProp("note", "third");
    test_equals(count_output_placeholders(&block), 3);
    test_equals(get_output_placeholder(&block, 0)->getProp("note"), "second");
    test_equals(get_output_placeholder(&block, 1)->getProp("note"), "third");
    test_equals(get_output_placeholder(&block, 2)->getProp("note"), "first");

    insert_output_placeholder(&block, NULL, 3)->setStringProp("note", "fourth");
    test_equals(count_output_placeholders(&block), 4);
    test_equals(get_output_placeholder(&block, 0)->getProp("note"), "second");
    test_equals(get_output_placeholder(&block, 1)->getProp("note"), "third");
    test_equals(get_output_placeholder(&block, 2)->getProp("note"), "first");
    test_equals(get_output_placeholder(&block, 3)->getProp("note"), "fourth");
}

void test_find_or_create_output_term()
{
    Block block;
    block.compile("def f() -> int { 5 }");
    Term* x = block.compile("x = f()");
    int originalLength = block.length();

    test_assert(find_or_create_output_term(x, 0) == x);
    test_equals(originalLength, block.length());

    Term* output1 = find_or_create_output_term(x, 1);
    test_assert(get_output_term(x, 1) == output1);
    test_equals(originalLength + 1, block.length());

    block.compile("def blah()");

    originalLength = block.length();
    Term* output2 = find_or_create_output_term(x, 2);
    test_assert(get_output_term(x, 1) == output1);
    test_assert(get_output_term(x, 2) == output2);
    test_equals(originalLength + 1, block.length());
}

void register_tests()
{
    REGISTER_TEST_CASE(building_test::test_insert_output_placeholder);
    REGISTER_TEST_CASE(building_test::test_find_or_create_output_term);
}

}
