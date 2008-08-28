
#include "common_headers.h"

#include "alien.h"
#include "branch.h"
#include "parser.h"
#include "tests/common.h"

class MyDumbType {
    std::string hi;
public:
    static int timesConstructorCalled;
    static int timesDestructorCalled;

    MyDumbType() {
        timesConstructorCalled++;
    }
    ~MyDumbType() {
        timesDestructorCalled++;
    }
};

int MyDumbType::timesConstructorCalled = 0;
int MyDumbType::timesDestructorCalled = 0;

namespace circa {
namespace alien_tests {

void test_simple() {
    Branch branch;

    MyDumbType::timesConstructorCalled = 0;
    alien::quick_create_type_templated<MyDumbType>(&branch, "MyDumbType");
    test_assert(MyDumbType::timesConstructorCalled == 0);

    parser::quick_eval_statement(&branch, "a = MyDumbType()");

    test_assert(MyDumbType::timesConstructorCalled == 1);
}

} // namespace alien_tests

void register_alien_tests()
{
    REGISTER_TEST_CASE(alien_tests::test_simple);
}

} // namespace circa
