// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "source_repro.h"

namespace source_repro_test {

void test_block_to_code_lines()
{
    Block block;
    block.compile("-- this is a comment\n    \n\nprint(1)");

    Value lines;
    block_to_code_lines(&block, &lines);

    test_equals(list_length(&lines), 4);
    test_equals(list_get(list_get(&lines, 0), 0), 1);
    test_equals(list_get(list_get(&lines, 0), 2), "-- this is a comment");
    test_equals(list_get(list_get(&lines, 1), 0), 2);
    test_equals(list_get(list_get(&lines, 1), 2), "    ");
    test_equals(list_get(list_get(&lines, 2), 0), 3);
    test_equals(list_get(list_get(&lines, 2), 2), "");
    test_equals(list_get(list_get(&lines, 3), 0), 4);
    test_equals(list_get(list_get(&lines, 3), 2), "print(1)");
}

void test_block_to_code_lines_multiline()
{
    Block block;
    block.compile("print(1\n  2\n3)");

    Value lines;
    block_to_code_lines(&block, &lines);

    test_equals(list_length(&lines), 3);
    test_equals(list_get(list_get(&lines, 0), 0), 1);
    test_equals(list_get(list_get(&lines, 0), 2), "print(1");
    test_equals(list_get(list_get(&lines, 1), 0), 2);
    test_equals(list_get(list_get(&lines, 1), 2), "  2");
    test_equals(list_get(list_get(&lines, 2), 0), 3);
    test_equals(list_get(list_get(&lines, 2), 2), "3)");
}

void register_tests()
{
    REGISTER_TEST_CASE(source_repro_test::test_block_to_code_lines);
    REGISTER_TEST_CASE(source_repro_test::test_block_to_code_lines_multiline);
}

} // namespace source_repro
