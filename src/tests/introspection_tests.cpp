
#include "common_headers.h"

#include "testing.h"
#include "builtins.h"
#include "introspection.h"
#include "syntax.h"
#include "values.h"

namespace circa {
namespace introspection_tests {

struct SourceReproResult {
    std::string expected;
    std::string actual;
    bool passed;
};

std::vector<SourceReproResult> gSourceReproResults;

void test_is_value()
{
    Branch branch;

    test_assert(is_value(create_value(&branch, INT_TYPE)));
    test_assert(is_value(create_value(&branch, STRING_TYPE)));
    test_assert(is_value(create_value(&branch, BOOL_TYPE)));
    test_assert(!is_value(branch.eval("1 + 2")));
}

void round_trip_source(std::string statement)
{
    SourceReproResult result;
    result.expected = statement;

    Branch branch;
    Term *t = branch.compile(statement);
    result.actual = get_term_source(t);
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
    round_trip_source("state int i");
    round_trip_source("state int b = 2");
    round_trip_source("  state int i");
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
    //round_trip_source("complex = (4.0 + 3.0) + 2.0");
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

void test_get_involved_terms()
{
    Branch branch;

    Term* a = branch.eval("a = 1.0");
    branch.eval("b = 2.0");
    Term* c = branch.eval("c = add(a,b)");
    Term* d = branch.eval("d = add(c,5.0)");
    branch.eval("e = add(c,5.0)");

    RefList subtree = get_involved_terms(RefList(a), RefList(d));

    test_equals(subtree, RefList(a,c,d));
}

void register_tests()
{
    REGISTER_TEST_CASE(introspection_tests::test_is_value);
    REGISTER_TEST_CASE(introspection_tests::reproduce_simple_values);
    REGISTER_TEST_CASE(introspection_tests::reproduce_stateful_values);
    REGISTER_TEST_CASE(introspection_tests::reproduce_function_calls);
    REGISTER_TEST_CASE(introspection_tests::reproduce_infix);
    REGISTER_TEST_CASE(introspection_tests::reproduce_rebinding_operator);
    REGISTER_TEST_CASE(introspection_tests::reproduce_dot_concat);
    REGISTER_TEST_CASE(introspection_tests::reproduce_if);
    REGISTER_TEST_CASE(introspection_tests::reproduce_lists);
    REGISTER_TEST_CASE(introspection_tests::test_get_involved_terms);
}

} // namespace introspection_tests

} // namespace circa
