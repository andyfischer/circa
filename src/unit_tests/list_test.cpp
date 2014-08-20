// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "list.h"

namespace list_test {

void set_list_1(Value* list, int x1)
{
    set_list(list, 0);
    set_int(list_append(list), x1);
}

void set_list_5(Value* list, int x1, int x2, int x3, int x4, int x5)
{
    set_list(list, 0);
    set_int(list_append(list), x1);
    set_int(list_append(list), x2);
    set_int(list_append(list), x3);
    set_int(list_append(list), x4);
    set_int(list_append(list), x5);
}

void test_sort()
{
    Value list;
    set_list_5(&list, 1, 2, 3, 4, 5);
    test_equals(&list, "[1, 2, 3, 4, 5]");
    list_sort_mergesort(&list, NULL, NULL);
    test_equals(&list, "[1, 2, 3, 4, 5]");

    set_list_5(&list, 5, 4, 3, 2, 1);
    list_sort_mergesort(&list, NULL, NULL);
    test_equals(&list, "[1, 2, 3, 4, 5]");

    set_list_5(&list, 1, 4, 3, 5, 2);
    list_sort_mergesort(&list, NULL, NULL);
    test_equals(&list, "[1, 2, 3, 4, 5]");

    set_list_5(&list, 3, 2, 2, 1, 1);
    list_sort_mergesort(&list, NULL, NULL);
    test_equals(&list, "[1, 1, 2, 2, 3]");

    set_list(&list, 0);
    list_sort_mergesort(&list, NULL, NULL);
    test_equals(&list, "[]");

    set_list_1(&list, 5);
    list_sort_mergesort(&list, NULL, NULL);
    test_equals(&list, "[5]");
}

int sort_backwards(void* context, Value* left, Value* right)
{
    test_equals((Value*) context, "context");
    return -1 * compare(left, right);
}

void test_sort_custom_func()
{
    Value context;
    set_string(&context, "context");

    Value list;
    set_list_5(&list, 1, 4, 3, 5, 2);
    list_sort_mergesort(&list, sort_backwards, &context);
    test_equals(&list, "[5, 4, 3, 2, 1]");
}

void register_tests()
{
    REGISTER_TEST_CASE(list_test::test_sort);
    REGISTER_TEST_CASE(list_test::test_sort_custom_func);
}

} // namespace list_test
