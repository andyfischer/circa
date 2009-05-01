// Copyright 2008 Andrew Fischer

#include <circa.h>

namespace circa {
namespace source_repro_tests {

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
    result.actual = get_branch_source(branch);
    result.passed = result.expected == result.actual;
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
        std::cout << "(desired string is on the left, actual result is on right)" << std::endl;
        for (unsigned i=0; i < gSourceReproResults.size(); i++) {
            SourceReproResult &result = gSourceReproResults[i];
            std::cout << (result.passed ? "[pass]" : "[FAIL]");
            std::cout << " \"" << result.expected << "\" ";
            std::cout << (result.passed ? "==" : "!=");
            std::cout << " \"" << result.actual << "\"" << std::endl;
        }
        declare_current_test_failed();
    }

    gSourceReproResults.clear();
}

// Source reproduction tests are grouped by category.
// When a test fails, we print the results of everything in that category, including
// things that succeeded. This is really helpful for debugging.

void reproduce_simple_values() {
    round_trip_source("1");
    round_trip_source("a = 1");
    round_trip_source("102");
    round_trip_source("-55");
    round_trip_source("0x102030");
    round_trip_source("   1");
    round_trip_source("1  ");
    round_trip_source("0.123");
    //round_trip_source(".123");
    //round_trip_source("-.123");
    round_trip_source("5.2");
    round_trip_source("5.200");
    finish_source_repro_category();
}

void reproduce_stateful_values() {
    round_trip_source("state i");
    round_trip_source("state i = 1");
    round_trip_source("state i = 5*3+1");
    //round_trip_source("state i:int");
    //round_trip_source("  state i:int");
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
    finish_source_repro_category();
}

void reproduce_rebinding_operator() {
    round_trip_source("a += 1");
    round_trip_source("a *= 5*3+1");
    round_trip_source("  a -= 5*3+1");
    round_trip_source("a /= 5*3+1  ");
    finish_source_repro_category();
}

void reproduce_dot_concat() {
    round_trip_source("'hello'.print");
    round_trip_source("   'hey'.print");
    round_trip_source("c = 'hello'.print");
    finish_source_repro_category();
}

void reproduce_if() {
    round_trip_source("if true\nx = 1\nend");
    round_trip_source("if 5.0 > 3.0\n  print('hey')\nend");
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
    finish_source_repro_category();
}

void reproduce_function_decl() {
    round_trip_source("def hi()\nend");
    round_trip_source("def hi2() : int\nend");
    round_trip_source("def hi3(int a)\nend");
    finish_source_repro_category();
}

void register_tests() {
    REGISTER_TEST_CASE(source_repro_tests::reproduce_simple_values);
    REGISTER_TEST_CASE(source_repro_tests::reproduce_stateful_values);
    REGISTER_TEST_CASE(source_repro_tests::reproduce_function_calls);
    REGISTER_TEST_CASE(source_repro_tests::reproduce_infix);
    REGISTER_TEST_CASE(source_repro_tests::reproduce_rebinding_operator);
    REGISTER_TEST_CASE(source_repro_tests::reproduce_dot_concat);
    REGISTER_TEST_CASE(source_repro_tests::reproduce_if);
    REGISTER_TEST_CASE(source_repro_tests::reproduce_lists);
    REGISTER_TEST_CASE(source_repro_tests::reproduce_for_loop);
    REGISTER_TEST_CASE(source_repro_tests::reproduce_function_decl);
}

} // namespace source_repro_tests
} // namespace circa
