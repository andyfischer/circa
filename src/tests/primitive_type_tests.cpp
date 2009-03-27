// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "circa.h"
#include "testing.h"

namespace circa {
namespace primitive_type_tests {

void strings()
{
    Branch branch;
    Term* str1 = string_value(&branch, "one");
    Term* str2 = string_value(&branch, "two");

    test_assert(as_string(str1) == "one");
    test_assert(as_string(str2) == "two");
    
    copy_value(str1,str2);

    test_assert(as_string(str1) == "one");
    test_assert(as_string(str2) == "one");
}

void builtin_objects()
{
    test_assert(find_named(KERNEL,"int") == INT_TYPE);
    test_assert(find_named(KERNEL,"float") == FLOAT_TYPE);
    test_assert(find_named(KERNEL,"string") == STRING_TYPE);
    test_assert(find_named(KERNEL,"bool") == BOOL_TYPE);
}

void register_tests()
{
    REGISTER_TEST_CASE(primitive_type_tests::strings);
    REGISTER_TEST_CASE(primitive_type_tests::builtin_objects);
}

} // namespace primitive_type_tests

} // namespace circa
