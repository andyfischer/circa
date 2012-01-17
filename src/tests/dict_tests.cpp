// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "dict.h"
#include "testing.h"

namespace circa {
namespace dict_tests {

void test_simple()
{
    TValue five;
    set_int(&five, 5);
    TValue ten;
    set_int(&ten, 10);

    DictData* data = dict_t::create_dict();
    dict_t::insert_value(&data, "a", &five);
    dict_t::insert_value(&data, "b", &ten);

    test_assert(dict_t::get_value(data, "a")->asInt() == 5);
    test_assert(dict_t::get_value(data, "b")->asInt() == 10);
    test_equals(dict_to_string(data), "{a: 5, b: 10}");
    dict_t::free_dict(data);
}

void test_insert()
{
    Dict dict;
    TValue v;
    set_int(&v, 5);

    TValue* a_inserted = dict.insert("a");
    test_equals(dict.toString(), "{a: null}");

    set_int(a_inserted, 7);
    test_equals(dict.toString(), "{a: 7}");
}

void dont_insert_same_key_multiple_times()
{
    DictData* data = dict_t::create_dict();

    TValue val;
    set_int(&val, 5);
    TValue val2;
    set_string(&val2, "a");

    dict_t::insert_value(&data, "key", &val);
    dict_t::insert_value(&data, "key", &val2);

    test_assert(dict_t::get_value(data, "key")->asString() == "a");
    test_assert(dict_t::count(data) == 1);
    dict_t::free_dict(data);
}


void handle_missing_keys()
{
    DictData* data = dict_t::create_dict();

    test_assert(dict_t::find_key(data, "a") == -1);
    test_assert(dict_t::get_value(data, "xyz") == NULL);

    dict_t::free_dict(data);
}

void hash_collision()
{
    // First, we need to figure out, for each hash bucket, a string which will
    // resolve to that bucket.
    //
    // This test doesn't assume knowledge of the hashing function, instead we'll
    // try a bunch of strings and see where they end up.

    DictData* dict = dict_t::create_dict();

    std::map<int, std::vector<std::string> > bucketToStr;

    for (char i = 0; i <= 26*3; i++) {
        char i_factored = i;
        char str[4];
        str[0] = 'a' + (i_factored % 26);
        i_factored /= 26;
        str[1] = 'a' + (i_factored % 26);
        i_factored /= 26;
        str[2] = 'a' + (i_factored % 26);
        i_factored /= 26;
        str[3] = 0;

        int bucket = dict_t::insert(&dict, str);

        bucketToStr[bucket].push_back(str);

        dict_t::clear(dict);
    }

    // Make sure that we found at least 2 strings for each of the first 5 buckets. If
    // we didn't then this test won't work.
    for (int i=0; i < 5; i++) {
        test_assert(bucketToStr.find(i) != bucketToStr.end());
    }

    // Test 1: Insert two colliding strings, make sure the second is redirected to a
    // different bucket, make sure we can lookup both strings, remove the first string,
    // make sure the redirected string is relocated.

    int bucket0 = dict_t::insert(&dict, bucketToStr[0][0].c_str());
    test_assert(bucket0 == 0);
    int bucket1 = dict_t::insert(&dict, bucketToStr[0][1].c_str());
    test_assert(bucket1 == 1);

    test_assert(dict_t::get_value(dict, bucketToStr[0][0].c_str()) != NULL);
    test_assert(dict_t::get_value(dict, bucketToStr[0][1].c_str()) != NULL);

    dict_t::remove(dict, bucketToStr[0][0].c_str());
    
    test_assert(dict_t::get_value(dict, bucketToStr[0][0].c_str()) == NULL);
    test_assert(dict_t::get_value(dict, bucketToStr[0][1].c_str()) != NULL);
    
    // Insert the first key again
    bucket0 = dict_t::insert(&dict, bucketToStr[0][0].c_str());
    test_assert(bucket0 == 1);

    test_assert(dict_t::get_value(dict, bucketToStr[0][0].c_str()) != NULL);
    test_assert(dict_t::get_value(dict, bucketToStr[0][1].c_str()) != NULL);

    dict_t::clear(dict);

    test_assert(dict_t::get_value(dict, bucketToStr[0][0].c_str()) == NULL);
    test_assert(dict_t::get_value(dict, bucketToStr[0][1].c_str()) == NULL);

    // Test 2: Insert a string in [0], insert a string in [1], then insert a string
    // which should go to [0] but gets redirected to [2]. Then delete the string in [1]
    // (so the string at [2] should go to [1] even though it wants to go to [0])
    bucket0 = dict_t::insert(&dict, bucketToStr[0][0].c_str());
    bucket1 = dict_t::insert(&dict, bucketToStr[1][0].c_str());
    int bucket2 = dict_t::insert(&dict, bucketToStr[0][1].c_str());

    test_assert(bucket0 == 0);
    test_assert(bucket1 == 1);
    test_assert(bucket2 == 2);

    test_assert(dict_t::get_value(dict, bucketToStr[0][0].c_str()) != NULL);
    test_assert(dict_t::get_value(dict, bucketToStr[1][0].c_str()) != NULL);
    test_assert(dict_t::get_value(dict, bucketToStr[0][1].c_str()) != NULL);
    
    dict_t::remove(dict, bucketToStr[1][0].c_str());

    test_assert(dict_t::get_value(dict, bucketToStr[0][0].c_str()) != NULL);
    test_assert(dict_t::get_value(dict, bucketToStr[1][0].c_str()) == NULL);
    test_assert(dict_t::get_value(dict, bucketToStr[0][1].c_str()) != NULL);

    dict_t::clear(dict);

    // Test 3: Similar to the previous test. Insert a string at [0], insert at [1],
    // and insert a string that wants to go to [0] but gets redirected to [2]. Then
    // delete the item at [0]. Now the item at [2] should get moved to [0], but
    // will the hash look that far?
    
    test_assert(dict_t::insert(&dict, bucketToStr[0][0].c_str()) == 0);
    test_assert(dict_t::insert(&dict, bucketToStr[1][0].c_str()) == 1);
    test_assert(dict_t::insert(&dict, bucketToStr[0][1].c_str()) == 2);

    test_assert(dict_t::get_value(dict, bucketToStr[0][0].c_str()) != NULL);
    test_assert(dict_t::get_value(dict, bucketToStr[1][0].c_str()) != NULL);
    test_assert(dict_t::get_value(dict, bucketToStr[0][1].c_str()) != NULL);
    
    dict_t::remove(dict, bucketToStr[0][0].c_str());

    test_assert(dict_t::get_value(dict, bucketToStr[0][0].c_str()) == NULL);
    test_assert(dict_t::get_value(dict, bucketToStr[1][0].c_str()) != NULL);
    test_assert(dict_t::get_value(dict, bucketToStr[0][1].c_str()) != NULL);

    // Disabled these tests, currently there's a bug where keys are not always relocated
    // as best they can be. This bug isn't causing any wrong behavior, it's just a
    // performance concern.
    
    // TEST_DISABLED test_assert(dict_t::insert(&dict, bucketToStr[0][1].c_str()) == 0);

    // Insert the first string again, now it should go to [2]
    // TEST_DISABLED test_assert(dict_t::insert(&dict, bucketToStr[0][0].c_str()) == 2);

    dict_t::free_dict(dict);
}

void many_items()
{
    const int count = 100;

    DictData* data = dict_t::create_dict();

    // Insert lots of items
    for (int i=0; i < count; i++) {
        char key[10]; sprintf(key, "%d", i);
        TValue val; set_int(&val, i);
        dict_t::insert_value(&data, key, &val);
    }

    test_assert(dict_t::count(data) == count);

    // Make sure they are really there
    for (int i=0; i < count; i++) {
        char key[10]; sprintf(key, "%d", i);
        test_assert(dict_t::get_value(data, key)->asInt() == i);
    }

    // Delete every odd item
    for (int i=0; i < count; i++) {
        if ((i % 2) == 0)
            continue;
        char key[10]; sprintf(key, "%d", i);
        dict_t::remove(data, key);
    }

    test_assert(dict_t::count(data) == count / 2);

    // Make sure the even items are accessible
    for (int i=0; i < count; i++) {
        char key[10]; sprintf(key, "%d", i);
        if ((i % 2) == 0) {
            // Even numbers should still be there
            test_assert(dict_t::get_value(data, key)->asInt() == i);
        } else {
            // Odd numbers should be gone
            test_assert(dict_t::get_value(data, key) == NULL);
        }
    }

    dict_t::free_dict(data);
}

void test_duplicate()
{
    TValue eleven = TValue::fromInt(11);
    TValue one_and_change = TValue::fromFloat(1.2);
    TValue t = TValue::fromBool(true);
    TValue hello = TValue::fromString("hello");

    DictData* data = dict_t::create_dict();
    dict_t::insert_value(&data, "a", &eleven);
    dict_t::insert_value(&data, "b", &one_and_change);
    dict_t::insert_value(&data, "c", &t);

    test_equals(dict_to_string(data), "{a: 11, b: 1.2, c: true}");

    DictData* dupe = dict_t::duplicate(data);

    test_equals(dict_to_string(dupe), "{a: 11, b: 1.2, c: true}");

    // Modify original, make sure that dupe is unaffected
    dict_t::remove(data, "b");
    dict_t::insert_value(&data, "d", &hello);
    test_equals(dict_to_string(data), "{a: 11, c: true, d: 'hello'}");
    test_equals(dict_to_string(dupe), "{a: 11, b: 1.2, c: true}");

    dict_t::free_dict(data);
    dict_t::free_dict(dupe);
}

void test_reset()
{
    Dict dict;

    TValue a;
    set_int(&a, 4);
    dict.set("a", &a);

    test_equals(dict.toString(), "{a: 4}");
    reset(&dict);
    test_equals(dict.toString(), "{}");
    dict.set("a", &a);
    test_equals(dict.toString(), "{a: 4}");
}

void test_iterate()
{
    Dict dict;
    TValue iterator;

    TValue one;
    set_int(&one, 1);
    TValue two;
    set_int(&two, 2);

    dict.set("one", &one);
    dict.set("two", &two);

    const char* currentKey;
    TValue* currentTValue;

    bool foundOne = false;
    bool foundTwo = false;
    for (dict.iteratorStart(&iterator);
            !dict.iteratorFinished(&iterator);
            dict.iteratorNext(&iterator)) {

        dict.iteratorGet(&iterator, &currentKey, &currentTValue);

        test_assert(currentTValue != NULL);

        if (std::string(currentKey) == "one") {
            test_assert(!foundOne);
            test_equals(currentTValue->asInt(), 1);
            foundOne = true;
        } else if (std::string(currentKey) == "two") {
            test_assert(!foundTwo);
            test_equals(currentTValue->asInt(), 2);
            foundTwo = true;
        } else {
            test_assert(false);
        }
    }
    test_assert(foundOne);
    test_assert(foundTwo);
}

void test_delete_from_iterator()
{
    Dict dict;
    TValue iterator;

    const char* names[] = {"a","b","c","d","e"};

    for (int i=0; i < 5; i++) {
        TValue val;
        set_int(&val, i);
        dict.set(names[i], &val);
    }

    test_equals(&dict, "{a: 0, b: 1, c: 2, d: 3, e: 4}");

    for (dict.iteratorStart(&iterator);
            !dict.iteratorFinished(&iterator);
            dict.iteratorNext(&iterator)) {

        const char* currentKey;
        TValue* currentTValue;

        dict.iteratorGet(&iterator, &currentKey, &currentTValue);

        if ((as_int(currentTValue) % 2) == 1) {
            dict.iteratorDelete(&iterator);
            test_assert(is_null(currentTValue));
        }
    }

    test_equals(&dict, "{a: 0, c: 2, e: 4}");
}

void test_cpp_wrapper()
{
    Dict dict;
    set_int(dict.insert("a"), 1);
    set_int(dict.insert("b"), 2);
    test_equals(dict.get("a"), "1");
    test_assert(dict.getInt("a", -1) == 1);
    test_assert(dict.getInt("b", -1) == 2);
    test_assert(dict.getInt("c", -1) == -1);
}

void register_tests()
{
    REGISTER_TEST_CASE(dict_tests::test_simple);
    REGISTER_TEST_CASE(dict_tests::test_insert);
    REGISTER_TEST_CASE(dict_tests::dont_insert_same_key_multiple_times);
    REGISTER_TEST_CASE(dict_tests::handle_missing_keys);
    REGISTER_TEST_CASE(dict_tests::hash_collision);
    REGISTER_TEST_CASE(dict_tests::many_items);
    REGISTER_TEST_CASE(dict_tests::test_duplicate);
    REGISTER_TEST_CASE(dict_tests::test_reset);
    REGISTER_TEST_CASE(dict_tests::test_iterate);
    REGISTER_TEST_CASE(dict_tests::test_delete_from_iterator);
    REGISTER_TEST_CASE(dict_tests::test_cpp_wrapper);
}

} // namespace dict_tests
} // namespace circa
