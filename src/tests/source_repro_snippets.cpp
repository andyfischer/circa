// Copyright 2008 Paul Hodge

// source_repro_snippets.cpp
//
// In this test suite, we do source reproduction on a bunch of example snippets of
// code, and we make sure that the reproduced string is exactly the same as the
// original string.
//
// Snippets are organized by category, and when any element in a category fails, we
// display everything in that category. In practice, it's very helpful to see the
// successful cases along side the failing cases.

#include <circa.h>

namespace circa {
namespace source_repro_snippets {

struct SourceReproResult {
    std::string expected;
    std::string actual;
    bool passed;
};

std::vector<SourceReproResult> gSourceReproResults;

std::string convert_real_newlines_to_escaped(std::string s)
{
    std::stringstream out;

    for (unsigned i=0; i < s.length(); i++) {
        if (s[i] == '\n')
            out << "\\n";
        else
            out << s[i];
    }
    return out.str();
}

void round_trip_source(std::string statement)
{
    SourceReproResult result;
    result.expected = statement;

    Branch branch;
    parser::compile(&branch, parser::statement_list, statement);
    result.actual = get_branch_source(branch);
    result.passed = result.expected == result.actual;
    result.actual = convert_real_newlines_to_escaped(result.actual);
    result.expected = convert_real_newlines_to_escaped(result.expected);
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
    round_trip_source("'string with single quotes'");
    round_trip_source("\"string with double quotes\"");
    finish_source_repro_category();
}

void reproduce_boolean() {
    round_trip_source("true");
    round_trip_source("false");
    round_trip_source("a = true");
    round_trip_source("[false true false true]");
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
    round_trip_source("state i = 5*3+1");
    round_trip_source("state int i");
    round_trip_source("  state int i = 5");
    round_trip_source("state int i; i += 1");
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

void reproduce_rebinding_operator() {
    round_trip_source("a += 1");
    round_trip_source("a *= 5*3+1");
    round_trip_source("  a -= 5*3+1");
    round_trip_source("a /= 5*3+1  ");
    finish_source_repro_category();
}

void reproduce_if() {
    round_trip_source("if true\nx = 1\nend");
    round_trip_source("if true\n       1\nend");
    round_trip_source("if 5.0 > 3.0\n  print('hey')\nend");
    round_trip_source("if true\nelse\nend");
    round_trip_source("  if true\nelse\nend");
    round_trip_source("if true  \nelse\nend");
    round_trip_source("if true\n  else\nend");
    round_trip_source("if true\nelse  \nend");
    round_trip_source("if true\nelse\n  end");
    round_trip_source("if true\nelse\nend  ");
    round_trip_source("if true\nelif true\nend  ");
    round_trip_source("if true\n  elif true\nend");
    round_trip_source("if true\nelif true  \nend");
    round_trip_source("if true\nelif true\nelse\nend");
    round_trip_source("if true\nelif true\n  else\nend");
    round_trip_source("if 1 > 2\nprint('hi')\nelif 2 > 3\n  elif 3 > 4\nprint('hello')\nend");
    round_trip_source("if true 1 2 3 end");
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
    finish_source_repro_category();
}

void reproduce_for_loop() {
    round_trip_source("for x in range(1)\nend");
    round_trip_source("  for x in range(1)\nend");
    round_trip_source("for x in [5]\nend");
    round_trip_source("for x in [1  ,2 ;3 ]\nend");
    round_trip_source("for x in [1]\n   print(x)\nend");
    round_trip_source("l = [1]\nfor x in l\nend");
    round_trip_source("l = [1]\nfor x in l\n  x += 3\nend");
    round_trip_source("for x in [1];end");
    round_trip_source("for x in [1],end");
    round_trip_source("for x in [1]    ;end");
    round_trip_source("for x in [1];   end");
    round_trip_source("for x in [1]   end");
    round_trip_source("for x in [1] print(1)  end");
    finish_source_repro_category();
}

void reproduce_subroutine() {
    round_trip_source("def hi()\nend");
    round_trip_source("def hi2() : int\nend");
    round_trip_source("def hi2():int\nend");
    round_trip_source("def hi3(int a)\nend");
    round_trip_source("def hi4() : int\nreturn 1\nend");
    round_trip_source("def hi():int;return 1;end");
    round_trip_source("def hi():int  ;return 1;end");
    round_trip_source("def hi();1;end");
    round_trip_source("def hi() ;1;end");
    round_trip_source("def hi(); 1;end");
    round_trip_source("def hi()   end");
    round_trip_source("def hi() 1  end");
    round_trip_source("def hi(int)  end");
    round_trip_source("def hi(float, string, bool)  end");
    finish_source_repro_category();
}

void reproduce_type_decl() {
    round_trip_source("type mytype { int a }");
    round_trip_source("type mytype { int a, float b }");
    finish_source_repro_category();
}

void reproduce_do_once() {
    round_trip_source("do once\nend");
    round_trip_source("do once\nprint(1)\nend");
    round_trip_source("do once; print(1); end");
    round_trip_source("do once , print(1); end");
    round_trip_source("do once     end");
    round_trip_source("do once  1 2 3    end");
    finish_source_repro_category();
}

void reproduce_misc_blocks() {
    round_trip_source("namespace ns\nend");
    round_trip_source("namespace ns\nprint(1)\nend");
    round_trip_source("namespace ns end");
    round_trip_source("namespace ns 1 end");
    round_trip_source("namespace ns; print(1)\nend");
    round_trip_source("begin\nend");
    round_trip_source("begin;end");
    round_trip_source("begin;  ;end");
    round_trip_source("begin;  end");
    round_trip_source("blah = begin 1   end");
    finish_source_repro_category();
}

void reproduce_with_parse_errors() {
    round_trip_source("nonexistant_function()");
    round_trip_source("nonexistant_function(1 2 3)");
    round_trip_source("a.b");
    round_trip_source("a.b()");
    finish_source_repro_category();
}

void reproduce_dot_expressions() {
    round_trip_source("r = &1; r.name");
    round_trip_source("r = &1; r.asint");
    round_trip_source("r = &1; r.asint + 5");
    round_trip_source("l = []; l.append(1)");
    // These don't work because constructors are not reproduced
    //round_trip_source("type T { float x }; t = T(); t.x");
    //round_trip_source("type T { float x }; t = T(); t.x = 1.0");
    finish_source_repro_category();
}

void register_tests() {
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_simple_values);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_boolean);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_color_literal);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_stateful_values);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_function_calls);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_infix);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_rebinding_operator);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_if);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_lists);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_for_loop);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_subroutine);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_type_decl);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_do_once);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_misc_blocks);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_with_parse_errors);
    REGISTER_TEST_CASE(source_repro_snippets::reproduce_dot_expressions);
}

} // namespace source_repro_snippets
} // namespace circa
