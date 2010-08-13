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
    branch.compile("state i; i += 1");

    EvalContext context;
    evaluate_branch(&context, branch);

    std::cout << "state = " << context.topLevelState.toString() << std::endl;
}

void register_tests()
{
#ifdef BYTECODE
    REGISTER_TEST_CASE(bytecode_tests::test_print_simple);
    REGISTER_TEST_CASE(bytecode_tests::print_if_block);
    REGISTER_TEST_CASE(bytecode_tests::if_block_name_joining);
    REGISTER_TEST_CASE(bytecode_tests::test_evaluate);
    REGISTER_TEST_CASE(bytecode_tests::for_loop);
    REGISTER_TEST_CASE(bytecode_tests::top_level_state);
#endif
}

}
}
