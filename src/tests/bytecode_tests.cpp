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

    //std::cout << stack.toString() << std::endl;
}

void register_tests()
{
    REGISTER_TEST_CASE(bytecode_tests::test_print_simple);
    REGISTER_TEST_CASE(bytecode_tests::test_evaluate);
}

}
}
