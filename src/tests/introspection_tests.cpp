
#include "common_headers.h"

#include "testing.h"
#include "builtins.h"
#include "introspection.h"

namespace circa {
namespace introspection_tests {

void simple() {

    // print_branch_extended(*KERNEL, std::cout);

}

void reproduce_source() {
    Branch branch;

    Term *a = branch.apply("a = 1");
    test_assert(get_term_source(a) == "a = 1");

    //fixme
    //Term *b = branch.apply("b = add(1.0,2.0)");
    //std::cout << get_term_source(b) << std::endl;
    //test_assert(get_term_source(b) == "b = add(1.0,2.0)");

    Term *b = branch.apply("b = concat('a', 'b')");
    test_assert(get_term_source(b) == "b = concat('a', 'b')");

    Term *c = branch.apply("c = 'hello'.print");
    test_assert(get_term_source(c) == "c = 'hello'.print");
}

} // namespace introspection_tests

void register_introspection_tests()
{
    REGISTER_TEST_CASE(introspection_tests::simple);
    REGISTER_TEST_CASE(introspection_tests::reproduce_source);
}

} // namespace circa
