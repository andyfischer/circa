
#include "common_headers.h"

#include "cpp_interface.h"
#include "branch.h"
#include "parser.h"
#include "tests/common.h"

class Type1 {
public:
    static int instanceCount;

    std::string hi;

    Type1() {
        instanceCount++;
    }
    ~Type1() {
        instanceCount--;
    }
};

int Type1::instanceCount = 0;

namespace circa {
namespace cpp_interface_tests {

void test_simple() {
    Branch branch;

    Type1::instanceCount = 0;
    quick_create_cpp_type<Type1>(&branch, "Type1");

    test_assert(Type1::instanceCount == 0);
    Term* term = parser::quick_eval_statement(&branch, "a = Type1()");
    test_assert(Type1::instanceCount == 1);

    delete term;

    test_assert(Type1::instanceCount == 0);
}

} // namespace cpp_interface_tests

void register_cpp_interface_tests()
{
    REGISTER_TEST_CASE(cpp_interface_tests::test_simple);
}

} // namespace circa
