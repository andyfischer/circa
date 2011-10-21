// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa.h"
#include "testing.h"

namespace circa {
namespace primitive_type_tests {

void strings()
{
    Branch branch;
    Term* str1 = create_string(&branch, "one");
    Term* str2 = create_string(&branch, "two");

    test_assert(as_string(str1) == "one");
    test_assert(as_string(str2) == "two");
    
    copy(str1,str2);

    test_assert(as_string(str1) == "one");
    test_assert(as_string(str2) == "one");
}

void builtin_objects()
{
    test_assert(find_name(KERNEL,"int") == INT_TYPE);
    test_assert(find_name(KERNEL,"number") == FLOAT_TYPE);
    test_assert(find_name(KERNEL,"string") == STRING_TYPE);
    test_assert(find_name(KERNEL,"bool") == BOOL_TYPE);
}

void test_void()
{
    Branch branch;
    Term* v = create_void(&branch);
    test_equals(to_string(v), "<void>");
}

void float_to_string()
{
    Branch branch;

    // Assert that we retain the user's original string, no matter what ridiculous
    // formatting was used
    Term* a = branch.compile("-0.00000");
    test_assert(get_term_source_text(a) == "-0.00000");
    a = branch.compile(".01020");
    test_assert(get_term_source_text(a) == ".01020");
    a = branch.compile("00.00");
    test_assert(get_term_source_text(a) == "00.00");

    // Try changing a value, make sure that new value is printed
    Term* b = branch.compile(".1");
    set_float(b, .123456f);
    test_assert(get_term_source_text(b) == "0.123456");

    // Make sure that if we assign a float to a value which might get printed without
    // a decimal point, that we still do print the decimal point. Otherwise if the
    // string gets re-parsed, that value will have a different type.
    Term* c = branch.compile("1.0");
    set_float(c, 2.0);
    test_equals(get_term_source_text(c), "2.0");
}

void register_tests()
{
    REGISTER_TEST_CASE(primitive_type_tests::strings);
    REGISTER_TEST_CASE(primitive_type_tests::builtin_objects);
    REGISTER_TEST_CASE(primitive_type_tests::test_void);
    REGISTER_TEST_CASE(primitive_type_tests::float_to_string);
}

} // namespace primitive_type_tests

} // namespace circa
