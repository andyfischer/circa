// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include "circa.h"
#include "testing.h"

namespace circa {
namespace primitive_type_tests {

void strings()
{
    Branch branch;
    Term* str1 = create_string(branch, "one");
    Term* str2 = create_string(branch, "two");

    test_assert(as_string(str1) == "one");
    test_assert(as_string(str2) == "two");
    
    assign_value(str1,str2);

    test_assert(as_string(str1) == "one");
    test_assert(as_string(str2) == "one");
}

void builtin_objects()
{
    test_assert(find_named(*KERNEL,"int") == INT_TYPE);
    test_assert(find_named(*KERNEL,"number") == FLOAT_TYPE);
    test_assert(find_named(*KERNEL,"string") == STRING_TYPE);
    test_assert(find_named(*KERNEL,"bool") == BOOL_TYPE);
}

void test_void()
{
    Branch branch;
    Term* v = create_void(branch);
    test_equals(to_string(v), "<void>");
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

void float_to_string()
{
    Branch branch;

    // Assert that we retain the user's original string, no matter what ridiculous
    // formatting was used
    Term* a = branch.eval("-0.00000");
    test_assert(get_term_source_text(a) == "-0.00000");
    a = branch.eval(".01020");
    test_assert(get_term_source_text(a) == ".01020");
    a = branch.eval("00.00");
    test_assert(get_term_source_text(a) == "00.00");

    // Try changing a value, make sure that new value is printed
    Term* b = branch.eval(".1");
    set_float(b, .123456f);
    test_assert(get_term_source_text(b) == "0.123456");

    // Make sure that if we assign a float to a value which might get printed without
    // a decimal point, that we still do print the decimal point. Otherwise if the
    // string gets re-parsed, that value will have a different type.
    Term* c = branch.eval("1.0");
    set_float(c, 2.0);
    test_equals(get_term_source_text(c), "2.0");
}

void test_ref_tweak()
{
    Branch branch;
    Term* v = branch.eval("v = 1.0");
    test_assert(as_float(v) == 1.0); // sanity check
    
    // Check that no rounding errors are introduced
    branch.eval("r = &v");
    branch.eval("r.tweak(1)");
    branch.eval("r.tweak(-1)");
    test_assert(as_float(v) == 1.0);

    branch.eval("r.tweak(10)");
    for (int i=0; i < 10; i++)
        branch.eval("r.tweak(-1)");
    test_assert(as_float(v) == 1.0);
}

void register_tests()
{
    REGISTER_TEST_CASE(primitive_type_tests::strings);
    REGISTER_TEST_CASE(primitive_type_tests::builtin_objects);
    REGISTER_TEST_CASE(primitive_type_tests::test_void);
    REGISTER_TEST_CASE(primitive_type_tests::assign_int_to_float);
    REGISTER_TEST_CASE(primitive_type_tests::float_to_string);
    REGISTER_TEST_CASE(primitive_type_tests::test_ref_tweak);
}

} // namespace primitive_type_tests

} // namespace circa
