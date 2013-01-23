// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "block.h"
#include "control_flow.h"
#include "term.h"

namespace control_flow_test {

void simple_get_exit_rank()
{
    Block block;

    Term* ret = block.compile("return");
    Term* br = block.compile("break");
    Term* con = block.compile("continue");
    Term* dis = block.compile("discard");

    test_assert(term_get_highest_exit_level(ret) == sym_ExitLevelFunction);
    test_assert(term_get_highest_exit_level(br) == sym_ExitLevelLoop);
    test_assert(term_get_highest_exit_level(con) == sym_ExitLevelLoop);
    test_assert(term_get_highest_exit_level(dis) == sym_ExitLevelLoop);
}

void test_implicit_exit_points()
{
#if 0
    Block block;

    Term* a = block.compile("a = 1");
    update_exit_points(&block);
    test_assert(find_trailing_exit_point(a) == NULL);

    Term* ret = block.compile("return 1");
    update_exit_points(&block);
    test_assert(find_trailing_exit_point(ret) != NULL);

    Term* f = block.compile("for i in 0..3 { return 1 }");
    update_exit_points(&block);
    test_assert(find_trailing_exit_point(f) != NULL);

    // Double check that 'a' isn't messed up.
    test_assert(find_trailing_exit_point(a) == NULL);
#endif
}

void register_tests()
{
    REGISTER_TEST_CASE(control_flow_test::simple_get_exit_rank);
    REGISTER_TEST_CASE(control_flow_test::test_implicit_exit_points);
}

}
