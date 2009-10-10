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

    Term errorListener;
    evaluate_branch(code, &errorListener);

    if (errorListener.hasError()) {
        std::cout << "Runtime error in: " << get_current_test_name() << std::endl;
        print_runtime_error_formatted(code, std::cout);
        std::cout << std::endl;
        std::cout << branch_to_string_raw(code);
        declare_current_test_failed();
        return;
    }

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
    test_snippet("for i in [1] end", "");
    test_snippet("def func() end", "");
}

void test_rounding()
{
    test_snippet("", "round(.1) == 0");
    test_snippet("", "round(.4) == 0");
    test_snippet("", "round(.6) == 1");
    test_snippet("", "round(.9) == 1");
    test_snippet("", "round(1.2) == 1");
    test_snippet("", "round(-1.2) == -1");
    test_snippet("", "floor(.1) == 0");
    test_snippet("", "floor(.4) == 0");
    test_snippet("", "floor(.6) == 0");
    test_snippet("", "floor(.9) == 0");
    test_snippet("", "floor(1.2) == 1");
    test_snippet("", "floor(-1.2) == -2");
    test_snippet("", "ceil(.1) == 1");
    test_snippet("", "ceil(.4) == 1");
    test_snippet("", "ceil(.9) == 1");
    test_snippet("", "ceil(1.1) == 2");
    test_snippet("", "ceil(-1.1) == -1");
}

void test_boolean_ops()
{
    test_snippet("", "true and true)");
    test_snippet("", "not(true and false)");
    test_snippet("", "not(false and true)");
    test_snippet("", "not(false and false)");
    test_snippet("", "true or true");
    test_snippet("", "false or true");
    test_snippet("", "true or false");
    test_snippet("", "not(false or false)");
    test_snippet("", "true and true or false");
    test_snippet("", "false and true or true");
}

void test_for_loops()
{
    test_snippet("l = []; for i in 0..3; l.append(i); end", "l == [0 1 2]");
    test_snippet("a = [3 2 1]; b = [];for item in a; b.append(item); end", "b == [3 2 1]");
    test_snippet("a = [2 4]; b = [1 3];for item in a; b.append(item); end", "b == [1 3 2 4]");
    test_snippet("a = [1 2];for item in []; a.append(item); end", "a == [1 2]");
    test_snippet("a = [1 2];for i in a; i += 1; end", "a == [1 2]");
    test_snippet("a = [1 2];for i in @a; i += 1; end", "a == [2 3]");
    test_snippet("a = [1 2 3];for i in @a; discard; end", "a == []");
    //test_snippet("a = [1 2 3];for i in @a; if i == 1 discard end end", "a == [2 3]");
    //test_snippet("a = [1 2 3];for i in @a; if i == 2 discard end end", "a == [1 3]");
    //test_snippet("a = [1 2 3];for i in @a; if i == 3 discard end end", "a == [1 2]");
    //test_snippet("a = [1 2 3];for i in @a; i += 1 if i == 3 discard end end", "a == [2 4]");
}

void test_subscripting()
{
    test_snippet("l = [[[1]]]", "l[0][0][0] == 1");
    test_snippet("def return_point() : Point return [8 9] end", "return_point().x == 8");
}

void test_set()
{
    test_snippet("s = Set(); s.add(1)", "s.contains(1); to_string(s) == '{1}'");
    test_snippet("s = Set()", "not(s.contains(1))");
    test_snippet("s = Set(); s.add(1); s.add(2); s.remove(1)",
            "not(s.contains(1)); s.contains(2)");
    test_snippet("s = Set(); s.add(1); s.add(1)", "to_string(s) == '{1}'");
}

void test_map()
{
    test_snippet("m = Map(); m.add(1,2)", "m.contains(1); to_string(m) == '{1: 2}'");
}

void register_tests()
{
    REGISTER_TEST_CASE(test_snippets::equals_snippets);
    REGISTER_TEST_CASE(test_snippets::test_abs);
    REGISTER_TEST_CASE(test_snippets::test_filter);
    REGISTER_TEST_CASE(test_snippets::test_modulo);
    REGISTER_TEST_CASE(test_snippets::test_references);
    REGISTER_TEST_CASE(test_snippets::test_blocks);
    REGISTER_TEST_CASE(test_snippets::test_rounding);
    REGISTER_TEST_CASE(test_snippets::test_boolean_ops);
    REGISTER_TEST_CASE(test_snippets::test_for_loops);
    REGISTER_TEST_CASE(test_snippets::test_subscripting);
    REGISTER_TEST_CASE(test_snippets::test_set);
    REGISTER_TEST_CASE(test_snippets::test_map);
}

} // namespace test_snippets
} // namespace circa
