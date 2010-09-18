// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace bytecode_tests {

using namespace circa::bytecode;

void test_print_simple()
{
    Branch branch;
    branch.compile("a = 1 + 2");
    branch.compile("b = a + 3");
    branch.compile("trace(b)");
    update_bytecode(branch);

    std::stringstream strm;
    print_bytecode(strm, branch);
}

void print_if_block()
{
    Branch branch;
    branch.compile("if 2 < 1 trace(1) else trace(2) end");
    update_bytecode(branch);

    std::stringstream strm;
    print_bytecode(strm, branch);
}

void if_block_name_joining()
{
    Branch branch;
    branch.compile("if true if true if true a = 1 else a = 2 end else a = 2 end else a = 2 end trace(a)");
    update_bytecode(branch);

    branch.clear();
    branch.compile("if false a = 1 else if false a = 1 else if false a = 1 else a = 2 end end end trace(a)");
    update_bytecode(branch);

    branch.clear();
    branch.compile("if false elif false elif false elif false end");
    update_bytecode(branch);
}

void test_evaluate()
{
    Branch branch;
    branch.compile("a = 1");
    branch.compile("b = a + 1");
    branch.compile("c = b * 2");

    EvalContext cxt;
    List stack;
    
    update_bytecode(branch);
    evaluate_bytecode(&cxt, &branch._bytecode, &stack);

    test_assert(stack[0]->asInt() == 1);
    test_assert(stack[1]->asInt() == 1);
    test_assert(stack[2]->asInt() == 2);
    test_assert(stack[3]->asInt() == 2);
    test_assert(stack[4]->asInt() == 4);
}

void for_loop()
{
    Branch branch;
    branch.compile("for i in [1 2 3] some_function(i) end");
    update_bytecode(branch);

    EvalContext cxt;
    List stack;
    evaluate_bytecode(&cxt, &branch._bytecode, &stack);
}

void top_level_state()
{
    Branch branch;
    branch.compile("state int i; i = 1");

    EvalContext context;
    evaluate_branch(&context, branch);

    Dict* state = Dict::checkCast(&context.topLevelState);
    test_equals(state->toString(), "[i: 1]");

    branch.clear();
    reset(&context.topLevelState);
    branch.compile("state int i; i += 1");

    for (int i = 0; i < 4; i++)
        evaluate_branch(&context, branch);

    test_equals(state->toString(), "[i: 4]");
}

void if_block_state()
{
    Branch branch;

    branch.compile("if true state i; i = 4 else state j; j = 3; end");

    EvalContext context;
    evaluate_branch(&context, branch);
    test_equals(context.topLevelState.toString(), "[#if_block: [[i: 4], null]]");

    branch.clear();
    branch.compile("if false state i; i = 4 else state j; j = 3; end");
    evaluate_branch(&context, branch);
    test_equals(context.topLevelState.toString(), "[#if_block: [null, [j: 3]]]");
}

void for_block_state()
{
    Branch branch;
    branch.compile("for i in 0..5 state s; s = 1 end");

    update_bytecode(branch);

    EvalContext context;
    evaluate_branch(&context, branch);

    test_equals(context.topLevelState.toString(),
            "[#hidden_state_for_for_loop: [[s: 1], [s: 1], [s: 1], [s: 1], [s: 1]]]");
}

void test_do_once()
{
    Branch branch;
    branch.compile("do once print('hi') end");
    EvalContext context;
    //FIXME
    //for (int i=0; i < 5; i++)
    //    evaluate_branch(&context, branch);
}

void register_tests()
{
    REGISTER_TEST_CASE(bytecode_tests::test_print_simple);
    REGISTER_TEST_CASE(bytecode_tests::print_if_block);
    REGISTER_TEST_CASE(bytecode_tests::if_block_name_joining);
    REGISTER_TEST_CASE(bytecode_tests::test_evaluate);
    REGISTER_TEST_CASE(bytecode_tests::for_loop);
    REGISTER_TEST_CASE(bytecode_tests::top_level_state);
    REGISTER_TEST_CASE(bytecode_tests::if_block_state);
    REGISTER_TEST_CASE(bytecode_tests::for_block_state);
    REGISTER_TEST_CASE(bytecode_tests::test_do_once);
}

}
}
