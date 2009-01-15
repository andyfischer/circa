
#include "common_headers.h"

#include "testing.h"
#include "builtins.h"
#include "introspection.h"

namespace circa {
namespace introspection_tests {

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
    //round_trip_source("complex = (4.0 + 3.0) + 2.0");

    // whitespace tests
    round_trip_source("   1");
    round_trip_source("   5 + 4");
    round_trip_source("   concat('a', 'b')");
    round_trip_source("   'hey'.print");
}

} // namespace introspection_tests

void register_introspection_tests()
{
    REGISTER_TEST_CASE(introspection_tests::reproduce_source);
}

} // namespace circa
