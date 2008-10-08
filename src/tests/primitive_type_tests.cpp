// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "circa.h"
#include "tests/common.h"

namespace circa {
namespace primitive_type_tests {

void strings()
{
    Branch branch;
    Term* str1 = string_var(branch, "one");
    Term* str2 = string_var(branch, "two");

    test_assert(as_string(str1) == "one");
    test_assert(as_string(str2) == "two");
    
    duplicate_value(str1,str2);

    test_assert(as_string(str1) == "one");
    test_assert(as_string(str2) == "one");
}

void builtin_objects()
{
    test_assert(KERNEL->findNamed("int") == INT_TYPE);
    test_assert(KERNEL->findNamed("float") == FLOAT_TYPE);
    test_assert(KERNEL->findNamed("string") == STRING_TYPE);
    test_assert(KERNEL->findNamed("bool") == BOOL_TYPE);
}

} // namespace primitive_type_tests

void register_primitive_type_tests()
{
    REGISTER_TEST_CASE(primitive_type_tests::strings);
    REGISTER_TEST_CASE(primitive_type_tests::builtin_objects);
}

} // namespace circa
