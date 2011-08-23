// Copyright (c) Paul Hodge. See LICENSE file for license terms.

// In these tests, we have a bunch of "snippets" (small pieces of code). We execute
// the code and then check that a list of assertions are true.
//
// Each test should call the function test_snippet(code, assertions). Both inputs
// are strings which are compiled as Circa code. We compile 'code', then we compile
// 'assertions' as a subbranch in the 'code' branch. If there are any static errors,
// then the test fails. Then we go through the 'assertions' branch, and we look
// for any statements which return a boolean, and we make sure all of those have
// evaluated to true. (if any are false, the test fails).
//
// The 'code' section may be empty, you might do this if you can put the entire test
// inside 'assertions'. The 'assertions' section may be empty, you might do this if
// you're only interested in whether 'code' causes an error.

#include "common_headers.h"

#include <circa.h>

namespace circa {
namespace test_snippets {

bool check_that_terms_have_locations(Branch& branch, List* failures)
{
    bool allCorrect = true;
    for (int i=0; i < branch.length(); i++) {
        Term* term = branch[i];
        if (!term->sourceLoc.defined()) {
            List* failure = List::cast(failures->append(), 0);
            set_ref(failure->append(), term);
            set_string(failure->append(), get_term_source_text(term));
            allCorrect = false;
        }
    }
    return allCorrect;
}

void test_snippet(std::string codeStr, std::string assertionsStr)
{
    Branch code;
    parser::compile(code, parser::statement_list, codeStr);

    if (has_static_errors(code)) {
        std::cout << "In code snippet: " << codeStr << std::endl;
        print_static_errors_formatted(code, std::cout);
        dump(code);
        declare_current_test_failed();
        return;
    }

    std::stringstream checkInvariantsOutput;
    if (!branch_check_invariants_print_result(code, checkInvariantsOutput)) {
        std::cout << "Failed invariant in code: " << get_current_test_name() << std::endl;
        std::cout << checkInvariantsOutput.str();
        print_branch(std::cout, code);
        declare_current_test_failed();
        return;
    }

    Branch& assertions = create_branch(code, "assertions");
    parser::compile(assertions, parser::statement_list, assertionsStr);

    if (has_static_errors(assertions)) {
        std::cout << "In code snippet: " << assertionsStr << std::endl;
        print_static_errors_formatted(assertions, std::cout);
        declare_current_test_failed();
        dump(code);
        return;
    }

    EvalContext result;
    evaluate_branch(&result, code);

    if (result.errorOccurred) {
        std::cout << "Runtime error in: " << get_current_test_name() << std::endl;
        std::cout << "setup: " << codeStr << std::endl;
        std::cout << "assertion: " << assertionsStr << std::endl;
        print_runtime_error_formatted(result, std::cout);
        std::cout << std::endl;
        print_branch(std::cout, code);
        declare_current_test_failed();
        return;
    }

    checkInvariantsOutput.clear();
    if (!branch_check_invariants_print_result(assertions, checkInvariantsOutput)) {
        std::cout << "Failed invariant in assertions: " << get_current_test_name() << std::endl;
        std::cout << checkInvariantsOutput.str();
        print_branch(std::cout, code);
        declare_current_test_failed();
        return;
    }

    #if 0
    List sourceLocationFailures;
    if (!check_that_terms_have_locations(code, &sourceLocationFailures)) {
        std::cout << "Test snippet has terms missing locations in: "
            << get_current_test_name() << std::endl;
        std::cout << sourceLocationFailures.toString() << std::endl;
        print_branch(std::cout, code);
        declare_current_test_failed();
        return;
    }
    #endif

    int boolean_statements_found = 0;
    for (int i=0; i < assertions.length(); i++) {
        if (!is_statement(assertions[i]))
            continue;

        TaggedValue* result = get_local(assertions[i]);

        if (!is_bool(result))
            continue;

        boolean_statements_found++;

        if (!as_bool(result)) {
            std::cout << "In " << get_current_test_name() << std::endl;
            std::cout << "assertion failed: "
                << get_term_source_text(assertions[i]) << std::endl;
            std::cout << "Compiled code: " << std::endl;
            print_branch(std::cout, code);
            declare_current_test_failed();
            return;
        }
    }

    if (boolean_statements_found == 0 && assertionsStr != "") {
        std::cout << "In " << get_current_test_name() << std::endl;
        std::cout << "no boolean statements found in: " << assertionsStr << std::endl;
        print_branch(std::cout, code);
        declare_current_test_failed();
        return;
    }
}

void test_snippet_runtime_error(std::string const& str)
{
    Branch code;
    parser::compile(code, parser::statement_list, str);

    if (has_static_errors(code)) {
        std::cout << "In code snippet: " << str << std::endl;
        print_static_errors_formatted(code, std::cout);
        declare_current_test_failed();
        return;
    }

    EvalContext result;
    evaluate_branch(&result, code);

    if (!result.errorOccurred) {
        std::cout << "No runtime error occured: " << get_current_test_name() << std::endl;
        std::cout << str << std::endl;
        print_branch(std::cout, code);
        declare_current_test_failed();
        return;
    }
}

void test_strings()
{
    test_snippet("", "''.length() == 0");
    test_snippet("", "'abc'.length() == 3");
    test_snippet("", "'abc'.substr(0,0) == ''");
    test_snippet("", "'abc'.substr(0,1) == 'a'");
    test_snippet("", "'abc'.substr(1,2) == 'bc'");
    //test_snippet_runtime_error("'abc'.substr(-1,0)");
    //test_snippet_runtime_error("'abc'.substr(0,-1)");
    //test_snippet_runtime_error("'abc'.substr(4,0)");
    test_snippet("", "'abc'.slice(0,-1) == 'ab'");
    test_snippet("", "'abc'.slice(1,-1) == 'b'");
    test_snippet("", "'abc'.slice(0,1) == 'a'");
    test_snippet("", "'abc'.slice(0,3) == 'abc'");
    //test_snippet_runtime_error("'abc'.slice(4,0)");
    //test_snippet_runtime_error("'abc'.slice(0,5)");
    //test_snippet_runtime_error("'abc'.slice(0,-5)");
}

void test_null_literal()
{
    test_snippet("", "null == null");
    test_snippet("", "to_string(null) == 'null'");
}

void test_equals_snippets()
{
    test_snippet("", "4 == 4");
    test_snippet("", "4 == 4.0");
    test_snippet("", "4 != 4.5");
    test_snippet("", "4.0 == 4");
    test_snippet("", "4.5 != 4");
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
    test_snippet("", "equals(1,1)");
    test_snippet("", "not(equals(1,2))");
    test_snippet("", "equals('hello','hello')");
    test_snippet("", "not(equals('hello','goodbye'))");
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

void test_subroutine()
{
    test_snippet("def f() -> List { return([1]) }", "f() == [1]");
    test_snippet("def f()\n  return\nf()", "");
    test_snippet("def f()\nf()", "");
    test_snippet("def f() {}", "");
    test_snippet("def f() { return }", "");
    test_snippet("def f() { if 1==2 { return } }", "");

    // return a for list result
    test_snippet("def f()->List { return for i in 0..3 { i + 4 } }","f() == [4, 5, 6]");
    test_snippet("def f()->List { return (for i in 0..3 { i + 4 }) }","f() == [4, 5, 6]");
    test_snippet("def f()->List { return(for i in 0..3 { i + 4 }) }","f() == [4, 5, 6]");
}

void test_references()
{
    test_snippet("a = 1; ra = ref(a)", "ra.name() == 'a'");
    test_snippet("a = 1; ra = ref(a); rb = ref(a)", "ra == rb");

    // test source_location
    test_snippet("a = 1; ra = ref(a)", "ra.source_location() == [0, 1, 6, 1]");

    test_snippet(" a = 1; b = 2; c = 3; d = add(a b c); rd = ref(d)",
        "rd.inputs() == [ref(a) ref(b) ref(c)]");


#if 0
    test_snippet("br = begin; a = 1; state b = 2; 3; end;"
                 "bm = branch_ref(br); cons = bm.get_configs();"
                 "cons_0 = cons[0] -> Ref",
                 "length(cons) == 1; cons_0.name() == 'a'");

    // test .input
    test_snippet("a = 1; b = 2; c = add(a,b); c_ref = ref(c)",
            "c_ref.input(0) == ref(a); c_ref.input(1) == ref(b)");

    // test .length
    test_snippet("br = {1 2}; mir = branch_ref(br)",
            "mir.length() == 2");

    // test .get_index
    test_snippet("a = 1; b = 2; br = {a b}; mir = branch_ref(br);"
            "mir_0 = mir.get_index(0); mir_1 = mir.get_index(1)",
            "mir_0.asint() == 1; mir_1.asint() == 2");

    test_snippet("a = add(1 2 3); a_ref = ref(a)", "a_ref.num_inputs() == 3");
#endif
}

void test_blocks()
{
    test_snippet("if true {}", "");
    test_snippet("if true {} else {}", "");
    test_snippet("for i in [1] {}", "");
    test_snippet("def func() {}", "");
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
    test_snippet("", "true and true");
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

void test_cond()
{
    test_snippet("", "cond(true, 'a', 'b') == 'a'");
    test_snippet("", "cond(false, 'a', 'b') == 'b'");
    test_snippet("", "cond(true, 5, true) == 5");
    test_snippet("", "cond(false, 5, true) == true");
}

void test_if_block()
{
    test_snippet("a = 1; if true { a = 2 }", "a == 2");
    test_snippet("a = 1; if false { a = 2 }", "a == 1");
    test_snippet("a = 1; if true { a = 2 } else {}", "a == 2");
    test_snippet("a = 1; if false { a = 2 } else {}", "a == 1");
    test_snippet("a = 1; if true { a = 2 } else { a = 3 }", "a == 2");
    test_snippet("a = 1; if false { a = 2 } else { a = 3 }", "a == 3");
    test_snippet("a = 1; if true {} else { a = 3 }", "a == 1");
    test_snippet("a = 1; if false {} else { a = 3 }", "a == 3");
    test_snippet("a = 1; if false { a = 2 } elif true { a = 3 } else { a = 4 }", "a == 3");
    test_snippet("a = 1; if false { a = 2 } elif false { a = 3 } else { a = 4 }", "a == 4");
}

void test_for_loops()
{
    test_snippet("l = []; for i in 0..3 { int(@i), l.append(i) }", "l == [0 1 2]");
    test_snippet("a = [3 2 1]; b = [];for item in a { b.append(item) }", "b == [3 2 1]");
    test_snippet("a = [2 4]; b = [1 3];for item in a { b.append(item) }", "b == [1 3 2 4]");
    test_snippet("a = [1 2];for item in [] { a.append(item) }", "a == [1 2]");
    test_snippet("a = [1 2];for i in a { int(@i); i += 1 }", "a == [1 2]");
    test_snippet("a = [1 2];for i in @a { int(@i); i += 1 }", "a == [2 3]");
    test_snippet("a = [1 2 3];for i in @a { discard }", "a == []");

    test_snippet("a = []; if true { a.append(1) a.append(2) } for i in a { int(@i); add(i,i) }", "");

    test_snippet("a = [1 2 3];for i in @a { if i == 1 { discard } }", "a == [2 3]");
    test_snippet("a = [1 2 3];for i in @a { if i == 2 { discard } }", "a == [1 3]");
    test_snippet("a = [1 2 3];for i in @a { if i == 3 { discard } }", "a == [1 2]");
    test_snippet("a = [1 2 3];for i in @a { i += 1 if i == 3 { discard } }", "a == [2 4]");

    // For loop with state
    test_snippet("for i in [1 2 3] { state s = i }", "");

    // Syntax with significant indentation
    test_snippet("a = 0; for i in [1 2 3] a += i", "a == 6");
    test_snippet("a = []; for i in [1 2 3] a.append(i)", "a == [1 2 3]");

    // Name a for loop, even though it returns nothing
    test_snippet("my_for_loop = for i in [0] {}", "");
    test_snippet("def f() {}; my_for_loop = for i in [0] { f() }", "");
    test_snippet("a = 1; my_for_loop = for i in [0] { a = 2 }", "");
}

void test_for_loop_output()
{
    test_snippet("for_loop = for i in 0..4 { i + 4 }", "[4 5 6 7] == for_loop");
    test_snippet("for_loop = for i in 0..4 { add(i,2) sub(i,2) mult(i 2) }",
            "[0 2 4 6] == for_loop");
    test_snippet("for_loop = for i in 0..4 { mult(i 3) -- this is a comment }",
            "[0 3 6 9] == for_loop");
}

void test_subscripting()
{
    test_snippet("l = [[[1]]]", "l[0][0][0] == 1");
    test_snippet("def return_point() -> Point { return([8 9]) }", "return_point().x == 8");
}

void test_set()
{
    test_snippet("s = Set(); s.add(1)", "s.contains(1); to_string(s) == '{1}'");
    test_snippet("s = Set()", "not(s.contains(1))");
    test_snippet("s = Set(); s.add(1); s.add(2); s.remove(1)",
            "not(s.contains(1)); s.contains(2)");
    test_snippet("s = Set(); s.add(1); s.add(1)", "to_string(s) == '{1}'");
}

void test_map_type()
{
    test_snippet("m = Map(); m.add(1,2)", "m.contains(1); to_string(m) == '[1: 2]'");
    test_snippet("m = Map(); m.add(1,2); m.add(3,4)", "m.get(1) == 2; m.get(3) == 4");
    test_snippet("m = Map(); m.add('a','b')",
        "m.contains('a'); m.remove('a'); not(m.contains('a'))");
}

void test_field_syntax()
{
    test_snippet("p = Point(); p.x = 5.0; p.y = 3.0", "p == [5.0 3.0]");
}

void test_lexprs()
{
    // test index access
    test_snippet("l = [[[1]]]; l[0][0][0] = 2", "l == [[[2]]]");
    test_snippet("l = [[[1]]]; l[0][0] = 2", "l == [[2]]");
    test_snippet("l = [1]; l[0] = [[2]]", "l == [[[2]]]");

    // make sure listdatas are not improperly shared
    test_snippet("a = [1]; b = a; b[0] = 5", "a == [1]");
    test_snippet("a = [[1]]; b = a; b[0] = 5", "a == [[1]]");
    test_snippet("a = [[1]]; b = a[0]; b[0] = 5", "a == [[1]]");

    // test field access
}

void test_vectorized_funcs()
{
    test_snippet("", "[1 2] + 3 == [4 5]");
    test_snippet("", "[1 2] + 3.0 == [4.0 5.0]");
    test_snippet("", "[1 2] + [10 10] == [11 12]");
    test_snippet("", "[5 5 5] - [3 2 1] == [2 3 4]");
    test_snippet("", "[5 5 5] - 6 == [-1 -1 -1]");
    test_snippet("a = [2.0 2.0] -> Point; b = [4.0 4.0] -> Point", "a + b == [6.0 6.0]");
    test_snippet("a = [2.0 2.0] -> Point; b = [4.0 4.0]", "a + b == [6.0 6.0]");
    test_snippet("a = [2.0 2.0]; b = [4.0 4.0]", "a + b == [6.0 6.0]");
    test_snippet("a = [2.0 2.0]; b = [4.0 4.0] -> Point", "a + b == [6.0 6.0]");
    test_snippet("", "[1 1]*[1 1] == [1 1]");

    // This will cause an error if a - b can't be statically resolved to be a Point
    test_snippet("a = [1 1], b = [2 2]; norm(a - b)", "");
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
#if 0
    disabled, code segments no longer act like values
    test_snippet("b = { 1 2 3 }", "b[0] == 1, b[1] == 2, b[2] == 3");
    test_snippet("", "{ 1 } != 1"); // once caused a crash
    test_snippet("br = { 1 2 3 }; sum = 0; for i in br; sum += i end", "sum == 6");
#endif
}

void test_rebinding_operators()
{
    test_snippet("a = 1, a += 2", "a == 3");
    test_snippet("a = [1 1]->Point, a += [4 4]","a == [5.0 5.0]");

    // complex lexprs
    test_snippet("a = [1], a[0] += 2", "a == [3]");
    test_snippet("a = [1], a[0] -= 2", "a == [-1]");
    test_snippet("a = [1 1]->Point, a.x = 2", "a == [2.0 1.0]");
    test_snippet("a = [1 1]->Point, a.x += 2", "a == [3.0 1.0]");
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

void test_significant_indentation()
{
    test_snippet("namespace a\n  b = 5", "a:b == 5");
    test_snippet("l = 0..3, for i in @l\n int(@i)\n i += 3", "l == [3 4 5]");
    test_snippet("do once\n a = 5", "");
}

void test_concat()
{
    test_snippet("", "concat('a' 'b' 'c') == 'abc'");
    test_snippet("", "concat(\"hello \", \"world\") == \"hello world\"");
    test_snippet("", "concat(1) == '1'");
    test_snippet("", "concat() == ''");
}

void test_misc()
{
    test_snippet("    ", "");
    // These snippets once caused parser errors:
    test_snippet("add(1 1)    ", "");
    test_snippet("add(1 1)\n    ", "");
    test_snippet("l = []\nl.append([1])\n    ", "");
    test_snippet("def f() -> List { return [1.0] }; m = f(); m[0]->number","");
}

void test_methods1()
{
    test_snippet("s = '123'; s.length()", "");
    test_snippet("type T { string s }; v = T(); v.s.length()", "");
    test_snippet("type T { string s }; T().s.length()", "");

    // This once caused a failed assert in parser.cpp
    test_snippet("s = Set(); x = s.add(1)", "");
}

void test_methods2()
{
    test_snippet("def int.increment(i)->int { return i + 1 }", "1.increment() == 2");
    //test_snippet("def int.increment(i)->int { return i + 1 }", "int.increment(2) == 3");
    test_snippet("def Ref.name_w_suffix(s, string suffix)->string { return concat(s.name(),suffix) }",
            "a = 1; ref(a).name_w_suffix('_suff') == 'a_suff'");

    // Lookup a method inside a namespace.
    test_snippet("namespace ns { type T { int i } def T.str(s)->string { return concat(s.i) } }",
            "t = ns:T([1]); t.str() == '1'");
}

void test_lists()
{
    test_snippet("to_rect([0 0] [5.0 10.0])", "");
    test_snippet("", "filter([1 2 3] [true false true]) == [1 3]");
    test_snippet("", "[1 2 3].extend([4 5]) == [1 2 3 4 5]");

    // .insert
    test_snippet("", "[].insert(0, 5) == [5]");
    test_snippet("", "[1].insert(0, 5) == [5, 1]");
    test_snippet("", "[1 2 3].insert(1, 'hi') == [1 'hi' 2 3]");

    // .slice
    test_snippet("", "[].slice(0, 0) == []");
    test_snippet("", "[1].slice(0, 0) == []");
    test_snippet("", "[1].slice(0, 1) == [1]");
    test_snippet("", "[1 2 3].slice(1, 3) == [2 3]");
    test_snippet("", "[1 2 3].slice(0, 2) == [1 2]");
    test_snippet("", "[1 2 3].slice(0, 3) == [1 2 3]");
}

void test_type_check_functions()
{
    test_snippet("", "is_int(1)");
    test_snippet("", "not(is_int(1.0))");
    test_snippet("", "is_float(1.0)");
    test_snippet("", "not(is_float(1))");
    test_snippet("", "is_bool(true)");
    test_snippet("", "not(is_bool(1.0))");
    test_snippet("", "is_string('hello')");
    test_snippet("", "not(is_string(true))");
    test_snippet("", "is_list([1 2 3])");
    test_snippet("", "not(is_list(1.0))");
}

void test_namespace()
{
    test_snippet("namespace ns { a = 1 }", "ns:a == 1");

    // Namespaced type name in function definition.
    test_snippet("namespace ns { type t{} }; def f(ns:t t)", "");
    test_snippet("namespace ns { type t{int i} }; def f(ns:t val)->int { return val.i }",
            "f([1]) == 1");
}

void test_swap()
{
    test_snippet("a = 1; b = 's'; swap(&a &b)", "a == 's'; b == 1");
}

void test_subroutine_multiple_outputs()
{
    test_snippet("def f(int i :out) -> int { i += 7; return 2 }; a = 3; b = f(&a)",
            "a == 10, b == 2");
    test_snippet("def f(int i :out) { i += 7 }; a = 3; f(&a)", "a == 10");
    test_snippet("def f(int a :out, int b :out, int c :out)\n"
                 "  a += 1\n  b += 2\n  c += 3\n"
                 "a = 10; b = 10; c = 10;\n"
                 "f(&a,&b,&c)",
                 "a == 11, b == 12, c == 13");
    test_snippet("def f(int a :out, int b :out, int c :out)\n"
                 "  a += 1\n  b += 2\n  c += 3\n"
                 "a = 10; b = 10; c = 10;\n"
                 "f(a,&b,c)",
                 "a == 10, b == 12, c == 10");
    test_snippet("def f(int a :out) { a = 4; return; a = 6 }; a = 0; f(&a)", "a == 4");

    // this once caused an assert fail:
    test_snippet("def f(int a, int b, int c :out) {}; f(1 2 3)", "");
}

void test_recursion_and_multiple_outputs()
{
    test_snippet(
        "def tree(int depth, List leaves :out) {"
        "  leaves.append(depth)"
        
        "  if depth >= 3 {"
        "    return;"
        "  }"

        "  tree(depth + 1, &leaves) "
        "  tree(depth + 1, &leaves) "
        "} "
        "leaves = [] "
        "tree(0, &leaves)",

        "leaves == [0, 1, 2, 3, 3, 2, 3, 3, 1, 2, 3, 3, 2, 3, 3]"
        );
}

void test_left_arrow()
{
    test_snippet("a = (sqr <- 5)", "a == 25");
}

void test_stateful_value_decl()
{
    // There once was a bug where a state var that gets initialized to 'null' would have
    // a type of null, and so it wouldn't be able to change value.
    test_snippet("def f() { state a = null; a = 1 } f(); st = dump_scope_state()",
            "to_string(st) == '{_f: {a: 1}}'");
}

void test_dynamic_call()
{
    // No inputs/outputs
    test_snippet("def f() { }; f_ptr = f; dynamic_call(f_ptr, [])", "");

    // One input
    test_snippet("def f(int i) { }; f_ptr = f; dynamic_call(f_ptr, [1])", "");

    // One output
    test_snippet("def f()->int { return 5 }; f_ptr = f; ", "dynamic_call(f_ptr, []) == 5");

    // Input & output
    test_snippet("def f(int i)->int { return i+3 }; f_ptr = f; ",
            "dynamic_call(f_ptr, [10]) == 13");

    test_snippet("def f(int i)->int { return i + 1 } "
        "def g(int i)->int { return i // 2 } "
        "dispatch_result = for i in [4 5 6 7 8 9 10] { "
        "  if (i % 2 == 1) { x = f } else { x = g } "
        "  i = dynamic_call(x, [i]) }", "dispatch_result == [2, 6, 3, 8, 4, 10, 5]");
}

void test_multiple_name_assignment()
{
    test_snippet("a = b = c = 1", "a == 1; b == 1; c == 1");
}


void register_tests()
{
    REGISTER_TEST_CASE(test_snippets::test_strings);
    REGISTER_TEST_CASE(test_snippets::test_null_literal);
    REGISTER_TEST_CASE(test_snippets::test_equals_snippets);
    REGISTER_TEST_CASE(test_snippets::test_abs);
    REGISTER_TEST_CASE(test_snippets::test_filter);
    REGISTER_TEST_CASE(test_snippets::test_modulo);
    REGISTER_TEST_CASE(test_snippets::test_subroutine);
    REGISTER_TEST_CASE(test_snippets::test_references);
    REGISTER_TEST_CASE(test_snippets::test_blocks);
    REGISTER_TEST_CASE(test_snippets::test_rounding);
    REGISTER_TEST_CASE(test_snippets::test_boolean_ops);
    REGISTER_TEST_CASE(test_snippets::test_cond);
    REGISTER_TEST_CASE(test_snippets::test_if_block);
    REGISTER_TEST_CASE(test_snippets::test_for_loops);
    REGISTER_TEST_CASE(test_snippets::test_for_loop_output);
    REGISTER_TEST_CASE(test_snippets::test_subscripting);
    REGISTER_TEST_CASE(test_snippets::test_set);
    REGISTER_TEST_CASE(test_snippets::test_map_type);
    REGISTER_TEST_CASE(test_snippets::test_field_syntax);
    REGISTER_TEST_CASE(test_snippets::test_lexprs);
    REGISTER_TEST_CASE(test_snippets::test_vectorized_funcs);
    REGISTER_TEST_CASE(test_snippets::test_color_arithmetic);
    REGISTER_TEST_CASE(test_snippets::test_branch_value);
    REGISTER_TEST_CASE(test_snippets::test_rebinding_operators);
    REGISTER_TEST_CASE(test_snippets::test_repeat);
    REGISTER_TEST_CASE(test_snippets::test_range);
    REGISTER_TEST_CASE(test_snippets::test_significant_indentation);
    REGISTER_TEST_CASE(test_snippets::test_concat);
    REGISTER_TEST_CASE(test_snippets::test_misc);
    REGISTER_TEST_CASE(test_snippets::test_methods1);
    REGISTER_TEST_CASE(test_snippets::test_methods2);
    REGISTER_TEST_CASE(test_snippets::test_lists);
    REGISTER_TEST_CASE(test_snippets::test_type_check_functions);
    REGISTER_TEST_CASE(test_snippets::test_namespace);
    REGISTER_TEST_CASE(test_snippets::test_swap);
    REGISTER_TEST_CASE(test_snippets::test_subroutine_multiple_outputs);
    REGISTER_TEST_CASE(test_snippets::test_recursion_and_multiple_outputs);
    REGISTER_TEST_CASE(test_snippets::test_left_arrow);
    REGISTER_TEST_CASE(test_snippets::test_stateful_value_decl);
    REGISTER_TEST_CASE(test_snippets::test_dynamic_call);
    REGISTER_TEST_CASE(test_snippets::test_multiple_name_assignment);
}

} // namespace test_snippets
} // namespace circa
