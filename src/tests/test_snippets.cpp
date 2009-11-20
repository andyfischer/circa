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

        Term* term = *it;

        if (is_hidden(term)) {
            it.skipNextBranch();
            continue;
        }

        if (is_value(term) && is_branch(term) && !is_function(term)) {
            it.skipNextBranch();
            continue;
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
        std::cout << print_branch_raw(code);
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
            std::cout << print_branch_raw(code);
            declare_current_test_failed();
            return;
        }
    }

    if (boolean_statements_found == 0 && assertionsStr != "") {
        std::cout << "In " << get_current_test_name() << std::endl;
        std::cout << "no boolean statements found in: " << assertionsStr << std::endl;
        std::cout << print_branch_raw(code);
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
    test_snippet("a = 1; ra = &a", "ra.name() == 'a'");
    test_snippet("a = 1; ra = &a; rb = &a", "ra == rb");

    test_snippet("br = begin; a = 1; state b = 2; 3; end;"
                 "bm = branch_mirror(br); cons = bm.get_configs();"
                 "cons_0 = cons[0] :: Ref",
                 "length(cons) == 1; cons_0.name() == 'a'");

    // test .input
    test_snippet("a = 1; b = 2; c = add(a,b); c_ref = &c",
            "c_ref.input(0) == &a; c_ref.input(1) == &b");

    // test .length
    test_snippet("br = [1 2]; mir = branch_mirror(br)",
            "mir.length() == 2");

    // test .get_index
    test_snippet("a = 1; b = 2; br = [a b]; mir = branch_mirror(br);"
            "mir_0 = mir.get_index(0); mir_1 = mir.get_index(1)",
            "mir_0.asint() == 1; mir_1.asint() == 2");

    test_snippet("a = add(1 2 3); a_ref = &a", "a_ref.num_inputs() == 3");
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
    test_snippet("def return_point() :: Point return [8 9] end", "return_point().x == 8");
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
    test_snippet("m = Map(); m.add(1,2); m.add(3,4)", "m.get(1) == 2; m.get(3) == 4");
    test_snippet("m = Map(); m.add('a','b')",
        "m.contains('a'); m.remove('a'); not(m.contains('a'))");

    test_snippet("def f(int i)::int for i in [] return 0 end end; map(f, [0])", "");
}

void test_field_syntax()
{
    test_snippet("p = Point(); p.x = 5.0; p.y = 3.0", "p == [5.0 3.0]");
}

void test_vectorized_funcs()
{
    test_snippet("", "[1 2] + 3 == [4 5]");
    test_snippet("", "[1 2] + 3.0 == [4.0 5.0]");
    test_snippet("", "[1 2] + [10 10] == [11 12]");
    test_snippet("", "[5 5 5] - [3 2 1] == [2 3 4]");
    test_snippet("", "[5 5 5] - 6 == [-1 -1 -1]");
    test_snippet("a = [2.0 2.0] :: Point; b = [4.0 4.0] :: Point", "a + b == [6.0 6.0]");
    test_snippet("a = [2.0 2.0] :: Point; b = [4.0 4.0]", "a + b == [6.0 6.0]");
    test_snippet("a = [2.0 2.0]; b = [4.0 4.0]", "a + b == [6.0 6.0]");
    test_snippet("a = [2.0 2.0]; b = [4.0 4.0] :: Point", "a + b == [6.0 6.0]");
    test_snippet("", "[1 1]*[1 1] == [1 1]");
}

void test_color_arithmetic()
{
    test_snippet("", "#001122 == #001122");
    test_snippet("", "#001122ff == #001122");
    test_snippet("", "#001133 != #001122");
    //test_snippet("", "#ff0000 + (#0000ff - #ff0000) * 0.5 == #880088");
}

void test_branch_value()
{
    test_snippet("b = { 1 2 3 }", "b[0] == 1, b[1] == 2, b[2] == 3");
    //test_snippet("b = { apple = 1, bees = 2 }", "b.apple == 1, b.bees == 2");
}

void test_rebinding_operators()
{
    test_snippet("a = 1, a += 2", "a == 3");
    test_snippet("a = [1 1]::Point, a += [4 4]","a == [5.0 5.0]");
}

void test_get_field_and_set_field()
{
    test_snippet("type A{int z} type B{A y} type C{B x} w = [[[3]]]::C",
            "w.x.y.z == 3; get_field(w, 'x', 'y', 'z') == 3");
    test_snippet("type A{int z, int q} type B{A y} type C{B x} w = [[[3 4]]]::C",
            "set_field(w, 5, 'x', 'y', 'z') == [[[5 4]]]::C");
    //test_snippet("type A{int z, int q} type B{A y} type C{B x} w = [[[3 4]]]::C",
            //"w.x.y.z = 5; w == [[[5 4]]]::C");
    //test_snippet("type A{int z, int q} type B{A y} type C{B x} w = [[[3 4]]]::C",
            //"w.x.y.z += 1; w == [[[4 4]]]::C");
}

void test_repeat()
{
    test_snippet("", "repeat(1, 0) == []");
    test_snippet("", "repeat(1, 1) == [1]");
    test_snippet("", "repeat(true, 3) == [true true true]");
}

void test_range()
{
    test_snippet("", "0..3 == [0 1 2]");
    test_snippet("", "4..8 == [4 5 6 7]");
    test_snippet("", "3..0 == [3 2 1]");
    test_snippet("", "3..-2 == [3 2 1 0 -1]");
    test_snippet("", "0..0 == []");
    test_snippet("", "10..10 == []");
    test_snippet("", "25..26 == [25]");
    test_snippet("", "25..24 == [25]");
}

void test_stateful_code()
{
    test_snippet("def hi() state int i end; c = hi()", "inspect.get_state(c) == [0]");
    test_snippet("def hi() state int i; i = 4 end; c = hi()", "inspect.get_state(c) == [4]");
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
    REGISTER_TEST_CASE(test_snippets::test_field_syntax);
    REGISTER_TEST_CASE(test_snippets::test_vectorized_funcs);
    REGISTER_TEST_CASE(test_snippets::test_color_arithmetic);
    REGISTER_TEST_CASE(test_snippets::test_branch_value);
    REGISTER_TEST_CASE(test_snippets::test_rebinding_operators);
    REGISTER_TEST_CASE(test_snippets::test_get_field_and_set_field);
    REGISTER_TEST_CASE(test_snippets::test_repeat);
    REGISTER_TEST_CASE(test_snippets::test_range);
    REGISTER_TEST_CASE(test_snippets::test_stateful_code);
}

} // namespace test_snippets
} // namespace circa
