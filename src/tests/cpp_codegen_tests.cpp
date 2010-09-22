// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "cpp_codegen.h"

namespace circa {
namespace cpp_codegen_tests {

void test_simple()
{
    Branch branch;
    branch.compile("a = 1 + 2");

    cpp_codegen::CppWriter writer;
    cpp_codegen::write_branch_contents(writer, branch);

    test_equals(writer.result(), "int a = 1 + 2;");
}

void test_function()
{
    Branch branch;
    branch.compile("def f() -> int; print(1); print(2); return 4; end");

    cpp_codegen::CppWriter writer;
    cpp_codegen::write_branch_contents(writer, branch);

    test_equals(writer.result(), "int f()\n{\n    print(1);\n    print(2);\n    return(4);\n}");
}

void register_tests()
{
    REGISTER_TEST_CASE(cpp_codegen_tests::test_simple);
    REGISTER_TEST_CASE(cpp_codegen_tests::test_function);
}

}
}
