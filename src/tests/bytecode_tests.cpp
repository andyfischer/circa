// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace bytecode_tests {

void test_simple()
{
    Branch branch;
    branch.compile("a = 1 + 2");
    branch.compile("b = a + 3");
    branch.compile("trace(b)");
    bytecode::update_bytecode(branch);
    bytecode::print_bytecode(std::cout, branch);
}

void register_tests()
{
    REGISTER_TEST_CASE(bytecode_tests::test_simple);
}

}
}
