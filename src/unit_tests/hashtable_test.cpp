// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "hashtable.h"
#include "tagged_value.h"

namespace hashtable_test {

void using_empty_table()
{
    Value val, val2;
    set_hashtable(&val);

    copy(&val, &val2);
    touch(&val);
    hashtable_get(&val, &val2);
    set_null(&val);
}

void test_safe_copy()
{
    Value val1, val2;

    set_hashtable(&val1);
    set_int(hashtable_insert(&val1, temp_string("a")), 5);
    test_equals(&val1, "{'a': 5}");

    copy(&val1, &val2);
    test_equals(&val2, "{'a': 5}");
    test_assert(val1.value_data.ptr == val2.value_data.ptr);

    touch(&val1);
    set_int(hashtable_insert(&val1, temp_string("a")), 10);
    test_equals(&val1, "{'a': 10}");
    test_equals(&val2, "{'a': 5}");
    test_assert(val1.value_data.ptr != val2.value_data.ptr);
}

void test_mutable_hashtable_simple()
{
    Value val;
    set_mutable_hashtable(&val);
    set_int(hashtable_insert(&val, temp_string("a")), 5);
}

void test_mutable_hashtable()
{
    Value val1, val2;

    set_mutable_hashtable(&val1);
    set_int(hashtable_insert(&val1, temp_string("a")), 5);
    test_equals(&val1, "{'a': 5}");

    copy(&val1, &val2);
    touch(&val1);
    set_int(hashtable_insert(&val1, temp_string("a")), 10);
    test_equals(&val1, "{'a': 10}");
    test_equals(&val2, "{'a': 10}");
}

void register_tests()
{
    REGISTER_TEST_CASE(hashtable_test::using_empty_table);
    REGISTER_TEST_CASE(hashtable_test::test_safe_copy);
    REGISTER_TEST_CASE(hashtable_test::test_mutable_hashtable_simple);
    REGISTER_TEST_CASE(hashtable_test::test_mutable_hashtable);
}

} // namespace hashtable_test
