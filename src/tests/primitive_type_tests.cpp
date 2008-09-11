// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "circa.h"
#include "tests/common.h"

namespace circa {
namespace primitive_type_tests {

void strings()
{
    Branch* branch = new Branch();
    Term* str1 = constant_string(branch, "one");
    Term* str2 = constant_string(branch, "two");

    test_assert(as_string(str1) == "one");
    test_assert(as_string(str2) == "two");
    
    duplicate_value(str1,str2);

    test_assert(as_string(str1) == "one");
    test_assert(as_string(str2) == "one");
}

} // namespace primitive_type_tests

void register_primitive_type_tests()
{
    REGISTER_TEST_CASE(primitive_type_tests::strings);
}

} // namespace circa
