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

    test_equals(strm.str(),
            "stack_size 5\n"
            "push 1 -> 0\n"
            "push 2 -> 1\n"
            "call add_i(0 1) -> 2\n"
            "push 3 -> 3\n"
            "call add_i(2 3) -> 4\n"
            "call trace(4)\n");
}

void print_if_block()
{
    Branch branch;
    branch.compile("if 2 < 1 trace(1) else trace(2) end");
    update_bytecode(branch);

    std::stringstream strm;
    print_bytecode(strm, branch);

    test_equals(strm.str(),
            "stack_size 5\n"
            "push 2 -> 0\n"
            "push 1 -> 1\n"
            "call less_than_i(0 1) -> 2\n"
            "jump_if_not(2) offset:92\n"
            "push 1 -> 3\n"
            "call trace(3)\n"
            "jump offset:76\n"
            "push 2 -> 4\n"
            "call trace(4)\n");
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
    //print_bytecode(std::cout, branch);
    //evaluate_bytecode(branch);
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

void register_tests()
{
    REGISTER_TEST_CASE(bytecode_tests::test_print_simple);
    REGISTER_TEST_CASE(bytecode_tests::print_if_block);
    REGISTER_TEST_CASE(bytecode_tests::if_block_name_joining);
    REGISTER_TEST_CASE(bytecode_tests::test_evaluate);
}

}
}
