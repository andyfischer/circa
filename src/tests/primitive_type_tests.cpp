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
    
    assign_value(str1,str2);

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

void assign_int_to_float()
{
    Branch branch;
    Term* source = branch.eval("1");
    Term* dest = branch.eval("2.0");

    assign_value(source, dest);

    test_assert(is_float(dest));
    test_equals(as_float(dest), 1.0);
}

void register_tests()
{
    REGISTER_TEST_CASE(primitive_type_tests::strings);
    REGISTER_TEST_CASE(primitive_type_tests::builtin_objects);
    REGISTER_TEST_CASE(primitive_type_tests::assign_int_to_float);
}

} // namespace primitive_type_tests

} // namespace circa
