
#include "common_headers.h"

#include "testing.h"
#include "builtins.h"
#include "introspection.h"

namespace circa {
namespace introspection_tests {

void simple() {

    // print_branch_extended(*KERNEL, std::cout);

}

} // namespace introspection_tests

void register_introspection_tests()
{
    REGISTER_TEST_CASE(introspection_tests::simple);
}

} // namespace circa
