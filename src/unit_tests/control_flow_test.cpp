// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "block.h"
#include "building.h"
#include "control_flow.h"
#include "inspection.h"
#include "interpreter.h"
#include "term.h"

namespace control_flow_test {

void simple_get_exit_rank()
{
    Block block;

    Term* ret = block.compile("return");
    Term* br = block.compile("break");
    Term* con = block.compile("continue");
    Term* dis = block.compile("discard");

    test_assert(term_get_highest_exit_level(ret) == s_ExitLevelFunction);
    test_assert(term_get_highest_exit_level(br) == s_ExitLevelLoop);
    test_assert(term_get_highest_exit_level(con) == s_ExitLevelLoop);
    test_assert(term_get_highest_exit_level(dis) == s_ExitLevelLoop);
}

void has_control_flow_prop()
{
    Block block;

    Term* f_def = block.compile("def f() { if true { return } }");

    // Top-level block has no control flow, it shouldn't escape f().
    test_assert(block_get_bool_prop(&block, s_HasControlFlow, false) == false);

    // Inside "f" does have control flow.
    test_assert(true == block_get_bool_prop(
                find_block_from_path(&block, "f"), s_HasControlFlow, false));

    // Inside if-block does too.
    test_assert(true == block_get_bool_prop(
                find_block_from_path(&block, "f / function=if"),
                s_HasControlFlow, false));

    Term* returnCall = find_term_from_path(&block, "**/function=return");
    test_assert(block_get_bool_prop(returnCall->owningBlock, s_HasControlFlow, true));
}

void test_find_block_that_exit_point_will_reach()
{
    Block block;
    block.compile("def f1() { def f2() { if true { return }}}");

    Term* returnCall = find_term_from_path(&block, "** / function=return");
    Block* exitsTo = find_block_that_exit_point_will_reach(returnCall);
    test_assert(exitsTo == find_block_from_path(&block, "f1 / f2"));

    block.clear();
    block.compile("def f1() { def f2() { for i in [1] { if true { break } } } }");

    Term* breakCall = find_term_from_path(&block, "**/function=break");
    exitsTo = find_block_that_exit_point_will_reach(breakCall);
    test_assert(exitsTo == find_block_from_path(&block, "** / function=for"));
}

void register_tests()
{
    REGISTER_TEST_CASE(control_flow_test::simple_get_exit_rank);
    REGISTER_TEST_CASE(control_flow_test::has_control_flow_prop);
    REGISTER_TEST_CASE(control_flow_test::test_find_block_that_exit_point_will_reach);
}

}
