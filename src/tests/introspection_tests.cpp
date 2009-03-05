
#include "common_headers.h"

#include "testing.h"
#include "builtins.h"
#include "introspection.h"
#include "syntax.h"
#include "values.h"

namespace circa {
namespace introspection_tests {

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
    Branch branch;
    Term *t = branch.compile(statement);
    test_equals(get_term_source(t), statement);
}

void reproduce_source() {
    round_trip_source("1");
    round_trip_source("a = 1");
    round_trip_source("concat('a', 'b')");
    round_trip_source("b = concat('a', 'b')");
    round_trip_source("'hello'.print");
    round_trip_source("c = 'hello'.print");
    round_trip_source("add(1.0, 2.0)");
    round_trip_source("d = add(1.0, 2.0)");
    round_trip_source("1.0 + 2.0");
    round_trip_source("blah = 1.0 + 2.0");
    round_trip_source("coersion = 1 + 2");
    round_trip_source("complex = 1 + 2 + 3.0 + 4.0");
    round_trip_source("assert(false)");
    round_trip_source("if true\nx = 1\nend");
    round_trip_source("if 5.0 > 3.0\n  print('hey')\nend");
    round_trip_source("state int i");
    round_trip_source("state int b = 2");
    round_trip_source("102");
    round_trip_source("-55");
    round_trip_source("0x102030");
    //round_trip_source("complex = (4.0 + 3.0) + 2.0");

    // whitespace tests
    round_trip_source("   1");
    round_trip_source("   5 + 4");
    round_trip_source("   concat('a', 'b')");
    round_trip_source("   'hey'.print");
}

void register_tests()
{
    REGISTER_TEST_CASE(introspection_tests::test_is_value);
    REGISTER_TEST_CASE(introspection_tests::reproduce_source);
}

} // namespace introspection_tests

} // namespace circa
