// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "building.h"
#include "kernel.h"
#include "list.h"
#include "term.h"
#include "type.h"
#include "tagged_value.h"

namespace type_test {

void manually_create_compound_type()
{
    Type* type = create_compound_type();
    compound_type_append_field(type, TYPES.int_type, temp_string("a"));
    compound_type_append_field(type, TYPES.string, temp_string("b"));

    test_equals(type->header.refcount, 1);

    Value value;
    make(type, &value);
    test_equals(type->header.refcount, 2);

    test_equals(&value, "{a: 0, b: ''}");
    type_decref(type);
}

void reference_counting()
{
    Type* type = create_type();
    test_assert(type->header.refcount == 1);

    Value value;
    set_type(&value, type);
    test_assert(type->header.refcount == 2);

    set_null(&value);
    test_assert(type->header.refcount == 1);

    {
        Block block;
        Term* value = create_value(&block, type, "my_value");
        test_assert(type->header.refcount == 3);

        set_null(term_value(value));
        test_assert(type->header.refcount == 2);
    }

    test_assert(type->header.refcount == 1);

    set_type(&value, type);
    test_assert(type->header.refcount == 2);

    Value value2;
    copy(&value, &value2);
    test_assert(type->header.refcount == 3);

    copy(&value, &value2);
    test_assert(type->header.refcount == 3);
    
    type_decref(type);
}

void reference_counting_and_tagged_value()
{
    Type* type = create_type();
    test_assert(type->header.refcount == 1);

    Value value;
    make(type, &value);
    test_assert(type->header.refcount == 2);

    Value value2;
    copy(&value, &value2);
    test_assert(type->header.refcount == 3);
    copy(&value, &value2);
    test_assert(type->header.refcount == 3);

    set_null(&value2);
    test_assert(type->header.refcount == 2);
    set_null(&value);
    test_assert(type->header.refcount == 1);

    type_decref(type);
}

void reference_counting_and_cast()
{
    Type* type = create_compound_type();
    compound_type_append_field(type, TYPES.float_type, temp_string("a"));
    compound_type_append_field(type, TYPES.string, temp_string("b"));

    test_equals(type->header.refcount, 1);

    // Perform a cast that needs no conversion.
    Value incomingList;
    set_list(&incomingList, 2);
    set_float(list_get(&incomingList, 0), 2.0);
    set_string(list_get(&incomingList, 1), "apple");

    bool success = cast(&incomingList, type);
    test_assert(success);

    test_equals(type->header.refcount, 2);
    test_equals(&incomingList, "{a: 2.0, b: 'apple'}");

    set_null(&incomingList);
    test_equals(type->header.refcount, 1);

    // Perform a cast where an item needs conversion (from int to float)
    set_list(&incomingList, 2);
    set_int(list_get(&incomingList, 0), 2);
    set_string(list_get(&incomingList, 1), "apple");

    success = cast(&incomingList, type);
    test_assert(success);

    test_equals(&incomingList, "{a: 2.0, b: 'apple'}");

    type_decref(type);
}

void register_tests()
{
    REGISTER_TEST_CASE(type_test::manually_create_compound_type);
    REGISTER_TEST_CASE(type_test::reference_counting);
    REGISTER_TEST_CASE(type_test::reference_counting_and_tagged_value);
    REGISTER_TEST_CASE(type_test::reference_counting_and_cast);
}

} // namespace type_test
