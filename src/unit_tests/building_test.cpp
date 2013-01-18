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

void register_tests()
{
    REGISTER_TEST_CASE(building_test::test_insert_output_placeholder);
}

}
