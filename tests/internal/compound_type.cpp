// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "framework.h"
#include "kernel.h"
#include "list.h"
#include "type.h"

using namespace circa;

void build_compound_type()
{
    Type* type = create_compound_type();

    test_equals(&type->parameter, "[[], []]");

    compound_type_append_field(type, &STRING_T, "abc");

    test_equals(&type->parameter, "[[<Type string>], ['abc']]");

    compound_type_append_field(type, &INT_T, "def");

    test_equals(&type->parameter, "[[<Type string>, <Type int>], ['abc', 'def']]");

    test_assert(compound_type_get_field_type(type, 0) == &STRING_T);
    test_assert(compound_type_get_field_type(type, 1) == &INT_T);
}

void compound_type_register_tests()
{
    REGISTER_TEST_CASE(build_compound_type);
}
