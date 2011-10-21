// Copyright (c) Paul Hodge. See LICENSE file for license terms.

// source_repro_snippets.cpp
//
// In this test suite, we do source reproduction on a bunch of example snippets of
// code, and we make sure that the reproduced string is exactly the same as the
// original string.
//
// Snippets are organized by category, and when any element in a category fails, we
// display everything in that category. In practice, it's very helpful to see the
// successful cases along side the failing cases.

#include "circa.h"
#include "filesystem_dummy.h"

namespace circa {
namespace source_repro_snippets {

struct SourceReproResult {
    std::string expected;
    std::string actual;
    bool passed;
};

std::vector<SourceReproResult> gSourceReproResults;

void round_trip_source(std::string statement)
{
    SourceReproResult result;
    result.expected = statement;

    Branch branch;
    parser::compile(&branch, parser::statement_list, statement);

    StyledSource richSource;
    format_branch_source(&richSource, &branch);
    result.actual = unformat_rich_source(&richSource);

    result.passed = result.expected == result.actual;
    result.actual = escape_newlines(result.actual);
    result.expected = escape_newlines(result.expected);
    gSourceReproResults.push_back(result);
}

void finish_source_repro_category()
{
    // Check if there were any failures
    bool anyFailures = false;
    for (unsigned i=0; i < gSourceReproResults.size(); i++)
        if (!gSourceReproResults[i].passed)
            anyFailures = true;

    if (anyFailures) {
        std::cout << get_current_test_name() << " failed:" << std::endl;
        std::cout << "(actual result is on left, desired string is on right)" << std::endl;
        for (unsigned i=0; i < gSourceReproResults.size(); i++) {
            SourceReproResult &result = gSourceReproResults[i];
            std::cout << (result.passed ? "[pass]" : "[FAIL]");
            std::cout << " \"" << result.actual << "\" ";
            std::cout << (result.passed ? "==" : "!=");
            std::cout << " \"" << result.expected << "\"" << std::endl;
        }
        declare_current_test_failed();
    }

    gSourceReproResults.clear();
}

void reproduce_simple_values() {
    round_trip_source("1");
    round_trip_source("a = 1");
    round_trip_source("102");
    round_trip_source("-55");
    round_trip_source("0x102030");
    round_trip_source("   1");
    round_trip_source("1  ");
    round_trip_source("0.123");
    round_trip_source(".123");
    round_trip_source("-.123");
    round_trip_source("5.2");
    round_trip_source("5.200");
}

void reproduce_string_literal() {
    round_trip_source("'string with single quotes'");
    round_trip_source("\"string with double quotes\"");
    round_trip_source("<<<multiline string>>>");
    round_trip_source("\"string with escaped \\\"\\n\\' characters\"");
    finish_source_repro_category();
}

void reproduce_boolean() {
    round_trip_source("true");
    round_trip_source("false");
    round_trip_source("a = true");
    round_trip_source("[false true false true]");
    finish_source_repro_category();
}
void reproduce_null_value() {
    round_trip_source("null");
    round_trip_source("a = null");
    round_trip_source("[null null null]");
    finish_source_repro_category();
}

void reproduce_color_literal() {
    round_trip_source("#123");
    round_trip_source("#fafa");
    round_trip_source("#aabbcc");
    round_trip_source("#aabbccdd");
    round_trip_source("c = #aabbccdd");
    finish_source_repro_category();
}

void reproduce_stateful_values() {
    round_trip_source("state i");
    round_trip_source("state i = 1");
    round_trip_source("state i = 1  ");
    round_trip_source("state i = 5*3+1");
    round_trip_source("state int i");
    round_trip_source("  state int i = 5");
    round_trip_source("state int i; i += 1");
    round_trip_source("a = 1; state int a_copy = a");
    finish_source_repro_category();
}

void reproduce_function_calls() {
    round_trip_source("concat('a', 'b')");
    round_trip_source("   concat('a', 'b')");
    round_trip_source("b = concat('a', 'b')");
    round_trip_source("  b = concat('a', 'b')");
    round_trip_source("assert(false)");
    round_trip_source("add(1.0, 2.0)");
    round_trip_source("add(1, 2)");
    round_trip_source("add(1,2)");
    round_trip_source("add(1 2)");
    round_trip_source("add(1\n)");
    round_trip_source("add(1 2 3 4)");
    round_trip_source("add(   1   2,3 4  )");
    round_trip_source("  add(1,2)");
    round_trip_source("add(1,2)  ");
    round_trip_source("d = add(1.0, 2.0)");
    round_trip_source("  d = add(1.0, 2.0)");
    round_trip_source("  d   =   add(1 2)");
    round_trip_source("d= add(1 2)");
    round_trip_source("d=add(1 2)");
    round_trip_source("d=   add(1 2)");
    round_trip_source("    text_sprite = render_text(ui_font_medium, text, #000)");
    round_trip_source("def f(int a, int b, int c) { state s; }   f(1, 2 3)"); // once had a bug
    finish_source_repro_category();
}

void reproduce_rebound_input() {
    round_trip_source("a = 1; b = 2; swap(&a &b)");
    round_trip_source("a = 1; b = 2; swap(&a b)");
    round_trip_source("a = 1; b = 2; swap(  &a, b)");
    round_trip_source("a = 1; b = 2; swap(&a  , b)");
    finish_source_repro_category();
}

void reproduce_infix() {
    round_trip_source("1.0 + 2.0");
    round_trip_source("1.0 * 2.0");
    round_trip_source("1.0 / 2.0");
    round_trip_source("1.0 - 2.0");
    round_trip_source("blah = 1.0 + 2.0");
    round_trip_source("coersion = 1 + 2");
    round_trip_source("complex = 1 + 2 + 3.0 + 4.0");
    round_trip_source("   5 + 4");
    round_trip_source("5    + 4");
    round_trip_source("5 +    4");
    round_trip_source("5 + 4   ");
    round_trip_source("5+4");
    round_trip_source("complex = (4.0 + 3.0) + 2.0");
    //round_trip_source("complex =  (  4 + 3)  + 2.0");
    //round_trip_source("complex = (4 + 3  ) + 2.0");
    //round_trip_source("a.x");
    finish_source_repro_category();
}

void reproduce_methods() {
    round_trip_source("'string'.length()");
    round_trip_source("Map().add(1,2)");
    round_trip_source("Map().add(1, 2)");
    round_trip_source("Point().x = 1.0");
    //round_trip_source("unknown.what()");
    finish_source_repro_category();
}

void reproduce_rebinding() {
    round_trip_source("a += 1");
    round_trip_source("a *= 5*3+1");
    round_trip_source("  a -= 5*3+1");
    round_trip_source("a /= 5*3+1  ");
    round_trip_source("a.b += 1");
    round_trip_source("a.b   += 1");
    round_trip_source("a.b +=   1");
    round_trip_source("p = Point(); p.x += 1.0");
    round_trip_source("p = Point(); p.x   += 1.0");
    round_trip_source("p = Point(); p.x +=   1.0");
    round_trip_source("p = Point(); p.x = 1.0");
    round_trip_source("p = Point(); p.x   = 1.0");
    round_trip_source("p = Point(); p.x =   1.0");
    finish_source_repro_category();
}

void reproduce_if() {
    round_trip_source("if { true x = 1 }");
    round_trip_source("if true       { 1 }");
    round_trip_source("if true {     1 }");
    round_trip_source("if 5.0 > 3.0 { print('hey') }");
    round_trip_source("if true {} else {}");
    round_trip_source("  if true {} else {}");
    round_trip_source("if true   else {}");
    round_trip_source("if true   else {}");
    round_trip_source("if true else   {}");
    round_trip_source("if true else   {}");
    round_trip_source("if true else {}  ");
    round_trip_source("if true elif true {}  ");
    round_trip_source("if true   elif true {}");
    round_trip_source("if true elif true   {}");
    round_trip_source("if true elif true else {}");
    round_trip_source("if true elif true   else {}");
    round_trip_source("if 1 > 2 { print('hi') } elif 2 > 3 {}  elif 3 > 4 { print('hello') }");
    round_trip_source("if true { 1 2 3 }");
    round_trip_source("if true{1 2 3 }");
    round_trip_source("if true{ a = 1} else { a = 2 }; b = a"); // once had a bug
    finish_source_repro_category();
}

void reproduce_lists() {
    round_trip_source("[]");
    round_trip_source("  []");
    round_trip_source("[1]");
    round_trip_source("[1,2]");
    round_trip_source("[1 2]");
    round_trip_source("[ 1 2]");
    round_trip_source("[1 2 ]");
    round_trip_source("[1 , 2]");
    round_trip_source("[ 1 , 2 ]");
    round_trip_source(" [1,2]");
    round_trip_source("[1,2] ");
    round_trip_source("[1\n2\n3\n] ");
    round_trip_source("a = 1; [a]");
    round_trip_source("a = 1; [a a,a;a\na]");
    round_trip_source("a = [0 0] + [cos(1) sin(1)]");
    finish_source_repro_category();
}

void reproduce_for_loop() {
    round_trip_source("for x in 0..1 {}");
    round_trip_source("  for x in 0..1 {}");
    round_trip_source("for x in [5] {}");
    round_trip_source("for x in [1  ,2 ;3 ] {}");
    round_trip_source("for x in [1]\n  print(x)");
    round_trip_source("l = [1]\nfor x in l");
    round_trip_source("l = [1]\nfor x in l\n  x += 3");
    round_trip_source("for x in [1]   {}");
    round_trip_source("for x in [1] { print(1)  }");
    round_trip_source("for x in [1] { print(1) } ");
    round_trip_source("l = [1]; for x in @l { x += 1 }");
    round_trip_source("a = for x in [1] { x + 1 }");
    finish_source_repro_category();
}

void reproduce_subroutine() {
    round_trip_source("def hi() {}");
    round_trip_source("def hi2() -> int {}");
    round_trip_source("def hi2()->int {}");
    round_trip_source("def hi3(int a) {}");
    round_trip_source("def hi4() -> int\n  return(1)");
    round_trip_source("def hi()->int{return(1)}");
    round_trip_source("def hi()->int  {return(1)}");
    round_trip_source("def hi()->int { return(add(sub(1,1),1)) }");
    round_trip_source("def hi() {1}");
    round_trip_source("def hi()   {}");
    round_trip_source("def hi() { 1  }");
    round_trip_source("def hi(int)  {}");
    round_trip_source("def hi(int) {}");
    round_trip_source("def hi(int) {   }");
    round_trip_source("def hi(int) { 1 }");
    round_trip_source("def hi(number, string, bool)  {}");
    round_trip_source("type Point { number x, number y }\ndef hi() -> Point { return([0 0]) }");
    round_trip_source("def hi() { if true { return(1) } else { return(2) } }");
    round_trip_source("def hi() { x = 1 if true { return(x) } else { return(x) }}");
    round_trip_source("def my_func(int...) -> int {}");
    finish_source_repro_category();
}

void reproduce_subroutine_headers()
{
    round_trip_source("def f(int i) -> int;");
    //TEST_DISABLED round_trip_source("def f(NonexistantType i) -> int;");
    round_trip_source("def f(int i :out) -> int;");
    round_trip_source("def f(int i :meta) -> int;");
    // TODO: Test properties in any order
    finish_source_repro_category();
}

void reproduce_return_call() {
    round_trip_source("def f() { return 1; }");
    round_trip_source("def f() { return   1; }");
    round_trip_source("def f() { return(1) }");
    round_trip_source("def f() { return }");
    round_trip_source("def f() { return   }");
    finish_source_repro_category();
}

void reproduce_function_headers() {
    round_trip_source("def my_native +native ()");
    round_trip_source("def my_native +native ()   ");
    round_trip_source("def my_native +native (int)");
    round_trip_source("def my_native +native (int) -> int");
    round_trip_source("def myfunc(state int i)");
    finish_source_repro_category();
}

void reproduce_type_decl() {
    round_trip_source("type mytype { int a }");
    round_trip_source("type mytype { int a, number b }");
    round_trip_source("type mytype { int   a, number     b }");
    round_trip_source("type mytype { \n int a, number b }");
    round_trip_source("type mytype { int a\nnumber b }");
    round_trip_source("type mytype { int a,\nnumber b }");
    round_trip_source("type mytype { int a, number b \n}");
    round_trip_source("type mytype    { int a, number b }");
    round_trip_source("type mytype {   int a, number b }");
    round_trip_source("type mytype { int a, number b }    ");
    round_trip_source("type mytype {   } ");
    finish_source_repro_category();
}

void reproduce_do_once() {
    round_trip_source("do once {}");
    round_trip_source("do once\n print(1)");
    round_trip_source("do once { print(1) }");
    round_trip_source("do once     {}");
    round_trip_source("do once  { 1 2 3    }");
    finish_source_repro_category();
}

void reproduce_namespace() {
    round_trip_source("namespace ns {}");
    round_trip_source("namespace ns\n  print(1)");
    round_trip_source("namespace ns {}");
    round_trip_source("namespace ns { 1 }");
    round_trip_source("namespace ns { print(1) }");
    round_trip_source("namespace ns { a = 1 }; b = ns:a");
    round_trip_source("namespace ns { def what(){}} ns:what()");
}

void reproduce_namespace_and_include() {
    FakeFileSystem files;
    files["a"] = "namespace ns { a = 1; def f() }";

    //TEST_DISABLED round_trip_source("include('a'); ns:a");
    round_trip_source("include('a'); ns:f()");
}
void reproduce_misc_blocks() {
    round_trip_source("{}");
    round_trip_source("{  }");
    round_trip_source("{1}");
    round_trip_source("blah = {1}");
    round_trip_source("blah = { print(1) print(2) }");
    finish_source_repro_category();
}

void reproduce_with_parse_errors() {
    round_trip_source("nonexistant_function()");
    round_trip_source("nonexistant_function(1 2 3)");
    round_trip_source("a:b");
    round_trip_source("a:b()");
    finish_source_repro_category();
}

void reproduce_dot_expressions() {
    round_trip_source("r = ref(1); r.name()");
    round_trip_source("r = ref(1); r.asint()");
    round_trip_source("r = ref(1); r.asint() + 5");
    round_trip_source("l = []; l.append(1)");
    round_trip_source("t = Point(); t.x");
    round_trip_source("t = Point(); t.x = 1.0");
    round_trip_source("t = Point(); a = t.x");
    round_trip_source("type A{int z} type B{A y} x = B(); x.y.z");
    //round_trip_source("type A{int z} type B{A y} x = B(); x.y.z = 5");
    finish_source_repro_category();
}

void reproduce_unary() {
    round_trip_source("a = 1; -a");
    round_trip_source("a = 1.0; -a");
    finish_source_repro_category();
}

void reproduce_bracket_syntax() {
    round_trip_source("a = [1]; a[0]");
    round_trip_source("a = [1]; b = a[0]");
    finish_source_repro_category();
}

void reproduce_identifiers_inside_namespaces() {
    round_trip_source("namespace a { namespace b { c = 1 } } add(4, a:b:c)");
    round_trip_source("namespace a { namespace b { c = [1] } }; print(a:b:c[0])");
    round_trip_source("namespace a { namespace b { c = 1 } } [1 2 a:b:c]");
    round_trip_source("namespace a { namespace b { c = 1 } } state i = a:b:c");
    finish_source_repro_category();
}

void reproduce_namespaced_function() {
    round_trip_source("namespace ns { def func() {} } ns:func()");
    //round_trip_source("namespace ns { def func(int i) {} } 1 -> ns:func");
    finish_source_repro_category();
}

void reproduce_rebind_operator() {
    round_trip_source("a = 1; add(@a, 2)");
    finish_source_repro_category();
}

void reproduce_discard_statement() {
    round_trip_source("l = [1]; for i in l { discard }");
    finish_source_repro_category();
}

void reproduce_branch_styles() {
    round_trip_source("def hi():\n return(1)\nhi()");
    round_trip_source("def hi()->int:\n return(1)\nhi()");
    round_trip_source("def hi() { 1 2 3 }");
    finish_source_repro_category();
}

void reproduce_type_cast() {
    round_trip_source("number(1)");
    round_trip_source("Point([4 3])");
    round_trip_source("1 -> number");
    finish_source_repro_category();
}

void reproduce_uncallable_functions() {
    round_trip_source("blah()");
    round_trip_source("blee:blah()");
    round_trip_source("blue:blee:blah()");
    round_trip_source("1()");
    round_trip_source("()()()");
    finish_source_repro_category();
}

void reproduce_left_arrow() {
    round_trip_source("print <- 1");
    round_trip_source("print    <- 1");
    round_trip_source("print <-   1");
    round_trip_source("print <- {\n 1 1 1 }");
    finish_source_repro_category();
}

void register_tests() {
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_simple_values);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_string_literal);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_boolean);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_null_value);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_color_literal);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_stateful_values);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_function_calls);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_rebound_input);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_infix);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_methods);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_rebinding);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_if);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_lists);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_for_loop);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_subroutine);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_subroutine_headers);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_return_call);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_function_headers);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_type_decl);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_do_once);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_namespace);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_namespace_and_include);
    //TEST_DISABLED REGISTER_TEST_CASE(source_repro_snippets::reproduce_misc_blocks);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_with_parse_errors);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_dot_expressions);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_unary);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_bracket_syntax);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_identifiers_inside_namespaces);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_namespaced_function);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_rebind_operator);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_discard_statement);
    //TEST_DISABLED REGISTER_TEST_CASE(source_repro_snippets::reproduce_branch_styles);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_type_cast);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_uncallable_functions);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_left_arrow);
}

} // namespace source_repro_snippets
} // namespace circa
