// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "block.h"
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

void test_find_block_that_exit_point_will_reach()
{
    Block block;
    block.compile("def f1() { def f2() { if true { return }}}");

    Term* returnCall = find_term_from_path_expression(&block, "** / function=return");
    Block* exitsTo = find_block_that_exit_point_will_reach(returnCall);
    test_assert(exitsTo == find_block_from_path_expression(&block, "f1 / f2"));

    block.clear();
    block.compile("def f1() { def f2() { for i in [1] { if true { break } } } }");

    Term* breakCall = find_term_from_path_expression(&block, "**/function=break");
    exitsTo = find_block_that_exit_point_will_reach(breakCall);
    test_assert(exitsTo == find_block_from_path_expression(&block, "** / function=for"));
}

void test_recursion_with_state()
{
    Block block;
    block.compile("def f(int level) { state s = level; if level == 0 { return }; f(level - 1); }");
    block.compile("f(3)");

    Term* returnCall = find_term_from_path_expression(&block, "** / function=return");
    Block* f = find_block_from_path_expression(&block, "f");

    update_derived_inputs_for_exit_point(returnCall);
    test_assert(find_block_that_exit_point_will_reach(returnCall) == f);

    Stack stack;
    push_frame(&stack, &block);

    run_interpreter(&stack);

    Term* stateOutput = find_state_output(&block);
    caValue* stateValue = get_frame_register(top_frame(&stack), stateOutput);
    test_equals(stateValue, "{_f: {s: 3, _f: {s: 2, _f: {s: 1, _f: {s: 0, _f: null}}}}}");
}

void register_tests()
{
    REGISTER_TEST_CASE(control_flow_test::simple_get_exit_rank);
    REGISTER_TEST_CASE(control_flow_test::has_control_flow_prop);
    REGISTER_TEST_CASE(control_flow_test::test_find_block_that_exit_point_will_reach);
    REGISTER_TEST_CASE(control_flow_test::test_recursion_with_state);
}

}
