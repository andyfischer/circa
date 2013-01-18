// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "kernel.h"
#include "list.h"
#include "type.h"

namespace compound_type_test {

void build_compound_type()
{
    Type* type = create_compound_type();

    test_equals(&type->parameter, "[[], []]");

    compound_type_append_field(type, TYPES.string, "abc");

    test_equals(&type->parameter, "[[<Type String>], ['abc']]");

    compound_type_append_field(type, TYPES.int_type, "def");

    test_equals(&type->parameter, "[[<Type String>, <Type int>], ['abc', 'def']]");

    test_assert(compound_type_get_field_type(type, 0) == TYPES.string);
    test_assert(compound_type_get_field_type(type, 1) == TYPES.int_type);
}

void register_tests()
{
    REGISTER_TEST_CASE(compound_type_test::build_compound_type);
}

}
