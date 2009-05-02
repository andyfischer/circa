// Copyright 2008 Paul Hodge

#include <circa.h>

namespace circa {
namespace parser_error_tests {

struct TestInput
{
    std::string text;
    bool exceptionThrown;
    bool failedToCauseError;

    TestInput(std::string const& _text) : text(_text),
        exceptionThrown(false), failedToCauseError(false) {}
};

std::vector<TestInput> TEST_INPUTS;

void register_input(std::string const& in)
{
    TEST_INPUTS.push_back(TestInput(in));
}

// This function (hopefully) lists every possible parse error.
// These lines are interpreted as statement_lists. Many are errors simply because of an
// unexpected EOF.

void register_every_possible_parse_error()
{
    TEST_INPUTS.clear();

    register_input("def");
    register_input("def myfunc");
    register_input("def myfunc%");
    register_input("def myfunc(%");
    register_input("def myfunc(int %");
    register_input("def myfunc(int) %");
    register_input("def myfunc(int) : %");
    register_input("def myfunc(int)");
    register_input("type");
    register_input("type mytype");
    register_input("type mytype %");
    register_input("type mytype { %");
    register_input("type mytype { int %");
    register_input("type mytype { int a %");
    register_input("type mytype { int a");
    register_input("if");
    register_input("if true");
    register_input("if true else");
    register_input("if else");
    register_input("if true else else");
    register_input("if %");
    register_input("for");
    register_input("for %");
    register_input("for x");
    register_input("for x %");
    register_input("for x in");
    register_input("for x in %");
    register_input("for x in [1]");
    register_input("state");
    register_input("state %");
    register_input("state x = ");
    register_input("1 = 2");
    register_input("1 2 3 = 4");
    register_input("a.b 3 4 = 4");
    register_input("return");
    register_input("a.0");
    register_input("[].append %");
    register_input("[].append(%");
    register_input("[].append(");
    register_input("1 -> %");
    register_input("1 -> 2");
    register_input("(1 + 2");
    register_input("add %");
    register_input("add(");
    register_input("add( %");
    register_input("add(1");
    register_input("[");
}

void test_every_parse_error()
{
    register_every_possible_parse_error();

    std::vector<TestInput>::iterator it;

    bool anyHadProblems = false;

    for (it = TEST_INPUTS.begin(); it != TEST_INPUTS.end(); ++it) {
        TestInput &input = *it;

        Branch branch;

        try {
            parser::compile(&branch, parser::statement_list, input.text);
        } catch (std::exception const&) {
            input.exceptionThrown = true;
            anyHadProblems = true;
            continue;
        }

        if (count_compile_errors(branch) == 0) {
            input.failedToCauseError = true;
            anyHadProblems = true;
        }
    }

    if (anyHadProblems) {
        std::cout << "Encountered problems in parser_error_tests:" << std::endl;
        for (it = TEST_INPUTS.begin(); it != TEST_INPUTS.end(); ++it) {
            if (it->exceptionThrown)
                std::cout << "[EXCEPTION]";
            else if (it->failedToCauseError)
                std::cout << "[NO ERROR] ";
            else 
                std::cout << "[Success]  ";

            std::cout << " " << it->text << std::endl;
        }

        declare_current_test_failed();
    }
}

void register_tests()
{
    REGISTER_TEST_CASE(parser_error_tests::test_every_parse_error);
}

} // namespace parser_error_tests
} // namespace circa
