// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include <circa.h>

namespace circa {
namespace cpp_convenience_tests {

void test_accessor()
{
    Branch branch;
    Int a(branch, "a", 5);

    test_assert(a == 5);
    test_assert(branch.contains("a"));
    test_assert(branch["a"]->asInt() == 5);
    a = 3;
    test_assert(branch["a"]->asInt() == 3);

    Term* b = create_value(&branch, INT_TYPE, "b");
    as_int(b) = 8;
    Int b_accessor(branch, "b", 3);
    test_assert(b_accessor == 8);
    test_assert(as_int(b) == 8);

    b_accessor = 11;
    test_assert(b_accessor == 11);
    test_assert(as_int(b) == 11);
}

void register_tests()
{
    REGISTER_TEST_CASE(cpp_convenience_tests::test_accessor);
}

}
} // namespace circa
