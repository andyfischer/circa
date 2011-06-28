// Copyright (c) Paul Hodge. See LICENSE file for license terms.

// In this test, we have a bunch of snippets, where we try to reproduce every
// possible parser error.
//
// The goal of this test is to make sure that for every error: 1) no exception
// is thrown, and 2) it's detected as an error. 
//
// (1) is a pretty common bug, since the parsing code makes liberal use of ca_assert()
//

#include <circa.h>

namespace circa {
namespace parser_error_snippets {

struct TestInput
{
    std::string text;
    bool exceptionThrown;
    bool failedToCauseError;

    TestInput(std::string const& _text) : text(_text),
        exceptionThrown(false), failedToCauseError(false) {}
};

std::vector<TestInput> TEST_INPUTS;

void test_case(std::string const& in)
{
    TEST_INPUTS.push_back(TestInput(in));
}

void register_every_possible_parse_error()
{
    TEST_INPUTS.clear();

    test_case("def");
    test_case("def myfunc");
    test_case("def myfunc%");
    test_case("def myfunc(%");
    test_case("def myfunc(int %");
    test_case("def myfunc(int) %");
    test_case("def myfunc(int) -> %");
    test_case("def myfunc(foo, bar) : baz");
    test_case("type");
    test_case("type mytype");
    test_case("type mytype %");
    test_case("type mytype { %");
    test_case("type mytype { int %");
    test_case("type mytype { int a %");
    test_case("type mytype { int a");
    test_case("if");
    test_case("if else");
    test_case("if true else else");
    test_case("if %");
    test_case("for");
    test_case("for %");
    test_case("for x");
    test_case("for x %");
    test_case("for x in");
    test_case("for x in %");
    test_case("for x in 1\n");
    test_case("namespace");
    test_case("namespace %");
    test_case("state");
    test_case("state %");
    test_case("state x = ");
    test_case("state int x = ");
    test_case("state int %");
    test_case("state foo x");
    test_case("foo = 1; state foo x");
    test_case("a.b 3 4 = 4");
    test_case("a.0");
    test_case("[].append %");
    test_case("[].append(%");
    test_case("[].append(");
    test_case("1 -> %");
    test_case("1 -> 2");
    test_case("(1 + 2");
    test_case("add %");
    test_case("add(");
    test_case("add( %");
    test_case("add(1");
    test_case("[");
    test_case("a = []; a[");
    test_case("a = [1]; a[0");
    test_case("a()");
    test_case("a = 1; a()");
    test_case("def func() -> NonexistantType {}");
    test_case("nonexistant = nonexistant + 1");
    test_case("for i in 0..1 { print([0 [0 ]) }");
    test_case("a = 1; a = x");
    test_case("def NonexistantType.func()");
    test_case("def List%");
    test_case("def List.%");
    test_case("def List.func%");
    test_case("def qualified:name()");
}

void test_every_parse_error()
{
    register_every_possible_parse_error();

    std::vector<TestInput>::iterator it;

    int problemCount = 0;

    for (it = TEST_INPUTS.begin(); it != TEST_INPUTS.end(); ++it) {
        TestInput &input = *it;

        Branch branch;

        try {
            parser::compile(branch, parser::statement_list, input.text);
        } catch (std::exception const&) {
            input.exceptionThrown = true;
            problemCount++;
            continue;
        }

        if (!has_static_errors(branch)) {
            dump(branch);
            input.failedToCauseError = true;
            problemCount++;
        }
    }

    if (problemCount > 0) {
        std::cout << "Encountered " << problemCount << " problem";
        std::cout << (problemCount == 1 ? "" : "s");
        std::cout << " in parser_error_snippets:" << std::endl;
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
    REGISTER_TEST_CASE(parser_error_snippets::test_every_parse_error);
}

} // namespace parser_error_snippets
} // namespace circa
