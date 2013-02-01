// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "block.h"
#include "control_flow.h"
#include "inspection.h"
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

void has_control_flow_prop()
{
    Block block;

    Term* f_def = block.compile("def f() { if true { return } }");

    // Top-level block has no control flow, it shouldn't escape f().
    test_assert(block_get_bool_property(&block, sym_HasControlFlow, false) == false);

    // Inside "f" does have control flow.
    test_assert(true == block_get_bool_property(
                find_block_from_path_expression(&block, "f"), sym_HasControlFlow, false));

    // Inside if-block does too.
    test_assert(true == block_get_bool_property(
                find_block_from_path_expression(&block, "f / function=if_block"),
                sym_HasControlFlow, false));

    Term* returnCall = find_term_from_path_expression(&block, "**/function=return");
    test_assert(block_get_bool_property(returnCall->owningBlock, sym_HasControlFlow, true));
}

void register_tests()
{
    REGISTER_TEST_CASE(control_flow_test::simple_get_exit_rank);
    REGISTER_TEST_CASE(control_flow_test::has_control_flow_prop);
}

}
