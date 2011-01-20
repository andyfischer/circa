// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace recursion_tests {

void preserve_locals()
{
    // In this test we construct a recursive function, and we insert
    // some code segment inside it. The thing we test is that the
    // local variables from our code segment are correct after the
    // recursive call is made.
    
    std::vector<std::string> inputs;

    // each code segment should be such that 'a' is equal to 1
    inputs.push_back("a = 1"); // <-- simplest possible
    inputs.push_back("a = level+1");
    //TEST_DISABLED inputs.push_back("a = 'str'; b = level+1; swap(&a &b)");

    for (size_t i=0; i < inputs.size(); i++) {
        std::string input = inputs[i];

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
}

void register_tests()
{
    REGISTER_TEST_CASE(recursion_tests::preserve_locals);
}

} // namespace recursion_tests
} // namespace circa
