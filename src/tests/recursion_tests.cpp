// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <circa.h>

namespace circa {
namespace recursion_tests {

void do_preserve_locals_template(std::string input)
{
    Branch branch;
    branch.compile("def f(int level) -> any "
                   + input
                   + " if level==0 f(1) end return a end");
    test_assert(branch);

    TaggedValue* result = branch.eval("f(0)");

    if (result->toString() != "1") {
        std::cout << "recursion_tests::preserve_locals failed on input: "
            << input << std::endl;
        dump_branch(branch);
        declare_current_test_failed();
    }
}

void preserve_locals()
{
    // In this test we construct a recursive function, and we insert
    // some code segment inside it. The thing we test is that the
    // local variables from our code segment are correct after the
    // recursive call is made.
    
    // each code segment should be such that 'a' is equal to 1
    do_preserve_locals_template("a = 1"); // <-- simplest possible
    do_preserve_locals_template("a = level+1");
    do_preserve_locals_template("a = 'str'; b = level+1; swap(&a &b)");
    do_preserve_locals_template("a = 'x'; if true a = level+1 end");
    do_preserve_locals_template("a = level+1; for i in [] a = 'x' end");
    do_preserve_locals_template("a = 'x'; for i in [1 2] a = level+1 end");
}

void register_tests()
{
    REGISTER_TEST_CASE(recursion_tests::preserve_locals);
}

} // namespace recursion_tests
} // namespace circa
