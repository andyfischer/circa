// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

// In these tests, we have a bunch of "snippets" (small pieces of code). We execute
// the code and then check that a list of assertions are true.
//
// Each test should call the function test_snippet(code, assertions). Both inputs
// are strings which are compiled as Circa code. We compile 'code', then we compile
// 'assertions' as a subbranch in the 'code' branch. If there are any static errors,
// then the test fails. Then we go through the 'assertions' branch, and we look
// for any statements which return a boolean, and we make sure all of those have
// evaluated to true. (if any are false, the test fails)

#include "common_headers.h"

#include <circa.h>

namespace circa {
namespace test_snippets {

bool has_source_location_defined(Term* term)
{
    return term->hasProperty("colStart") && term->hasProperty("lineStart");
}

void test_snippet(std::string codeStr, std::string assertionsStr)
{
    Branch code;
    parser::compile(&code, parser::statement_list, codeStr);

    if (has_static_errors(code)) {
        std::cout << "In code snippet: " << codeStr << std::endl;
        print_static_errors_formatted(code, std::cout);
        declare_current_test_failed();
        return;
    }

    // Check that the input code had properly defined source locations
    for (BranchIterator it(code); !it.finished(); ++it) {
        if (is_hidden(*it)) { it.skipNextBranch(); continue; }

        if (!has_source_location_defined(*it)) {
            std::cout << "Missing source location:" << std::endl;
            std::cout << get_term_source(*it) << std::endl;
            std::cout << branch_to_string_raw(code) << std::endl;
            declare_current_test_failed();
        }
    }

    Branch& assertions = create_branch(code, "assertions");
    parser::compile(&assertions, parser::statement_list, assertionsStr);

    evaluate_branch(code);

    int boolean_statements_found = 0;
    for (int i=0; i < assertions.length(); i++) {
        if (!is_statement(assertions[i]))
            continue;

        if (!is_bool(assertions[i]))
            continue;

        boolean_statements_found++;

        if (!as_bool(assertions[i])) {
            std::cout << "In " << get_current_test_name() << std::endl;
            std::cout << "assertion failed: " << get_term_source(assertions[i]) << std::endl;
            std::cout << "Compiled code: " << std::endl;
            std::cout << branch_to_string_raw(code);
            declare_current_test_failed();
            return;
        }
    }

    if (boolean_statements_found == 0 && assertionsStr != "") {
        std::cout << "In " << get_current_test_name() << std::endl;
        std::cout << "no boolean statements found in: " << assertionsStr << std::endl;
        std::cout << branch_to_string_raw(code);
        declare_current_test_failed();
    }
}

void equals_snippets()
{
    test_snippet("", "4 == 4");
    test_snippet("", "3 != 4");
    test_snippet("", "4.0 == 4.0");
    test_snippet("", "4.0 != 5.0");
    test_snippet("", "'hello' == 'hello'");
    test_snippet("", "'hello' != 'bye'");
    test_snippet("", "true == true");
    test_snippet("", "false != true");
    test_snippet("", "[1 1] == [1 1]");
    test_snippet("", "[1 1] != [1 2]");
    test_snippet("", "[1 1 1] != [1 1]");
}


void test_abs()
{
    test_snippet("", "abs(7.0) == 7.0");
    test_snippet("", "abs(-7.0) == 7.0");
}

void test_filter()
{
    test_snippet("a = filter([], [])", "a == []");
    test_snippet("a = filter([1], [true])", "a == [1]");
    test_snippet("a = filter([1], [false])", "a == []");
    test_snippet("a = filter([1 2 3 4], [true false true false])", "a == [1 3]");
    test_snippet("a = filter([1 2 3 4], [false true false true])", "a == [2 4]");
}

void test_modulo()
{
    test_snippet("", "1 % 2 == 1");
    test_snippet("", "2 % 2 == 0");
    test_snippet("", "3 % 2 == 1");
    test_snippet("", "0 % 2 == 0");
    test_snippet("", "-1 % 2 == -1");
    test_snippet("", "mod(0, 2) == 0");
    test_snippet("", "mod(1, 2) == 1");
    test_snippet("", "mod(2, 2) == 0");
    test_snippet("", "mod(-1, 2) == 1");
    test_snippet("", "mod(0, 4) == 0");
    test_snippet("", "mod(-1, 4) == 3");
    test_snippet("", "mod(-2, 4) == 2");
}

void test_references()
{
    test_snippet("a = 1; ra = &a", "ra.name == 'a'");
    test_snippet("a = 1; ra = &a; rb = &a", "ra == rb");

    test_snippet("br = begin; a = 1; state b = 2; 3; end;"
                 "bi = [&br] : BranchInspector; cons = bi.get_configs();"
                 "cons_0 = cons[0] : Ref",
                 "length(cons) == 1; cons_0.name == 'a'");
}

void test_blocks()
{
    //test_snippet("if true end", "");
    //test_snippet("if true; else; end", "");
    //test_snippet("for i in [1] end", "");
    test_snippet("def func() end", "");
}

void register_tests()
{
    REGISTER_TEST_CASE(test_snippets::equals_snippets);
    REGISTER_TEST_CASE(test_snippets::test_abs);
    REGISTER_TEST_CASE(test_snippets::test_filter);
    REGISTER_TEST_CASE(test_snippets::test_modulo);
    REGISTER_TEST_CASE(test_snippets::test_references);
    REGISTER_TEST_CASE(test_snippets::test_blocks);
}

} // namespace test_snippets
} // namespace circa
