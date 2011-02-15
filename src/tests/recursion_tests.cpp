// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <circa.h>

namespace circa {
namespace recursion_tests {

void test_that_f_returns_1(std::string testName, std::string input)
{
    Branch branch;
    branch.compile(input);

    if (print_static_errors_formatted(branch, std::cout)) {
        dump(branch);
        declare_current_test_failed();
        return;
    }

    TaggedValue* result = branch.eval("f(0)");

    if (result->toString() != "1") {
        std::cout << "recursion_tests::" << testName << " failed on input: "
            << input << std::endl;
        dump(branch);
        std::cout << "output of f(0) = " << result->toString() << std::endl;
        declare_current_test_failed();
    }
}

void preserve_subroutine_local_template(std::string input)
{
    test_that_f_returns_1("preserve_subroutine_local",
            "def f(int level) -> any " + input + " if level==0 f(1) end return a end");
}

void preserve_subroutine_local()
{
    // In this test we construct a recursive function, and we insert
    // some code segment inside it. The thing we test is that the
    // local variables from our code segment are correct after the
    // recursive call is made.
    
    // each code segment should be such that 'a' is equal to 1
    preserve_subroutine_local_template("a = 1"); // <-- simplest possible
    preserve_subroutine_local_template("a = level+1");
    preserve_subroutine_local_template("a = 'str'; b = level+1; swap(&a &b)");
    preserve_subroutine_local_template("a = 'x'; if true a = level+1 end");
    preserve_subroutine_local_template("a = level+1; for i in [] a = 'x' end");
    preserve_subroutine_local_template("a = 'x'; for i in [1 2] a = level+1 end");
}

void preserve_if_local_template(std::string input)
{
    test_that_f_returns_1("preserve_if_local",
            "def f(int level) -> any "
              "a = 'bzz' "
              "if true "
                + input +
                " if level==0 f(1) end "
                "return a "
              "end "
            "end");
}

void preserve_if_local()
{
    preserve_if_local_template("a = 1");
    preserve_if_local_template("a = level+1");
    preserve_if_local_template("a = 'x' if true a = level+1 end");
}

void preserve_for_local_template(std::string input)
{
    test_that_f_returns_1("preserve_for_local",
            "def f(int level) -> any "
              "a = 'bzz' "
              "for i in [1] "
                + input + " "
                "if level==0 f(1) end "
                "return a "
              "end "
            "end");
}

void preserve_for_local()
{
    preserve_for_local_template("a = 1");
    preserve_for_local_template("a = level+1");
    preserve_for_local_template("a = for j in [2] a = 1 end");
}

void recursion_and_multiple_outputs()
{
    Branch branch;
    branch.compile("def f(int depth, List l +out) "
                     "if depth < 4 f(depth+1 &l) end "
                     "l.append(depth) "
                   "end "
                   "l = [] "
                   "f(0, &l)");

    evaluate_branch(branch);
}

void register_tests()
{
    REGISTER_TEST_CASE(recursion_tests::preserve_subroutine_local);
    REGISTER_TEST_CASE(recursion_tests::preserve_if_local);
    REGISTER_TEST_CASE(recursion_tests::preserve_for_local);
    REGISTER_TEST_CASE(recursion_tests::recursion_and_multiple_outputs);
}

} // namespace recursion_tests
} // namespace circa
