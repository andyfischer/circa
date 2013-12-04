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
    test_equals(&val1, "{a: 5}");

    copy(&val1, &val2);
    test_equals(&val2, "{a: 5}");
    test_assert(val1.value_data.ptr == val2.value_data.ptr);

    touch(&val1);
    set_int(hashtable_insert(&val1, temp_string("a")), 10);
    test_equals(&val1, "{a: 10}");
    test_equals(&val2, "{a: 5}");
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
    test_equals(&val1, "{a: 5}");

    copy(&val1, &val2);
    touch(&val1);
    set_int(hashtable_insert(&val1, temp_string("a")), 10);
    test_equals(&val1, "{a: 10}");
    test_equals(&val2, "{a: 10}");
}

void test_get_keys()
{
    Value table;
    set_hashtable(&table);
    set_int(hashtable_insert(&table, temp_string("a")), 1);
    set_int(hashtable_insert(&table, temp_string("b")), 2);
    set_int(hashtable_insert(&table, temp_string("c")), 3);

    Value keys;
    hashtable_get_keys(&table, &keys);
    test_equals(&keys, "['a', 'b', 'c']");
}

void test_hashtable_iterator()
{
    Value table;
    set_hashtable(&table);
    set_int(hashtable_insert(&table, temp_string("a")), 1);
    set_int(hashtable_insert(&table, temp_string("b")), 2);
    set_int(hashtable_insert(&table, temp_string("c")), 3);

    Value table2;
    set_hashtable(&table2);

    for (HashtableIterator it(&table); it; ++it) {
        set_value(hashtable_insert(&table2, it.currentKey()), it.current());
    }

    test_assert(equals(&table, &table2));
}

void register_tests()
{
    REGISTER_TEST_CASE(hashtable_test::using_empty_table);
    REGISTER_TEST_CASE(hashtable_test::test_safe_copy);
    REGISTER_TEST_CASE(hashtable_test::test_mutable_hashtable_simple);
    REGISTER_TEST_CASE(hashtable_test::test_mutable_hashtable);
    REGISTER_TEST_CASE(hashtable_test::test_get_keys);
    REGISTER_TEST_CASE(hashtable_test::test_hashtable_iterator);
}

} // namespace hashtable_test
