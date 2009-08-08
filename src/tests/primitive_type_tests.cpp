// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "common_headers.h"

#include "circa.h"
#include "testing.h"

namespace circa {
namespace primitive_type_tests {

void non_pointer_values()
{
    Branch branch;
    Term* i = branch.eval("1");
    test_assert((void*) &as_int(i) == &i->value);
    test_assert((void*) &as<int>(i) == &i->value);
}

void strings()
{
    Branch branch;
    Term* str1 = string_value(branch, "one");
    Term* str2 = string_value(branch, "two");

    test_assert(as_string(str1) == "one");
    test_assert(as_string(str2) == "two");
    
    assign_value(str1,str2);

    test_assert(as_string(str1) == "one");
    test_assert(as_string(str2) == "one");
}

void builtin_objects()
{
    test_assert(find_named(*KERNEL,"int") == INT_TYPE);
    test_assert(find_named(*KERNEL,"float") == FLOAT_TYPE);
    test_assert(find_named(*KERNEL,"string") == STRING_TYPE);
    test_assert(find_named(*KERNEL,"bool") == BOOL_TYPE);
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
    Term* a = branch.eval("a = -0.00000");
    test_assert(to_string(a) == "-0.00000");

    // Try changing a value, make sure that new value is printed
    Term* b = branch.eval("a = .1");
    b->asFloat() = .123456f;
    test_assert(to_string(b) == "0.123456");

    // Make sure that if we assign a float to a value which might get printed without
    // a decimal point, that we still do print the decimal point. Otherwise if the
    // string gets re-parsed, that value will have a different type.
    Term* c = branch.eval("a = 1.0");
    c->asFloat() = 2.0;
    test_equals(to_string(c), "2.0");
}

void register_tests()
{
    REGISTER_TEST_CASE(primitive_type_tests::non_pointer_values);
    REGISTER_TEST_CASE(primitive_type_tests::strings);
    REGISTER_TEST_CASE(primitive_type_tests::builtin_objects);
    REGISTER_TEST_CASE(primitive_type_tests::assign_int_to_float);
    REGISTER_TEST_CASE(primitive_type_tests::float_to_string);
}

} // namespace primitive_type_tests

} // namespace circa
