// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <circa.h>

#include "type_inference.h"

namespace circa {
namespace type_inference_tests {

void test_find_common_type()
{
    test_assert(find_common_type(&INT_T,&INT_T) == &INT_T);
    test_assert(find_common_type(&FLOAT_T,&FLOAT_T) == &FLOAT_T);
    test_assert(find_common_type(&INT_T,&FLOAT_T) == &FLOAT_T);
    test_assert(find_common_type(&BOOL_T,&STRING_T) == &ANY_T);
    test_assert(find_common_type(as_type(KERNEL->get("Point")),
                as_type(KERNEL->get("Rect"))) == LIST_TYPE);
}

void test_find_type_of_get_index()
{
    Branch branch;
    Term* range = branch.compile("range(4,5)");

    test_equals(find_type_of_get_index(range)->name, "int");

    Term* list1 = branch.compile("['hello' 'hi' 'bye']");
    test_equals(find_type_of_get_index(list1)->name, "string");

    Term* list2 = branch.compile("[0 -1 11]");
    test_equals(find_type_of_get_index(list2)->name, "int");

    Term* list3 = branch.compile("[]");
    test_equals(find_type_of_get_index(list3)->name, "any");

    Term* list4 = branch.compile("[0.1 4]");
    test_equals(find_type_of_get_index(list4)->name, "number");

    branch.compile("type T { number x, number y }");
    Term* list5 = branch.compile("[.1 .1] -> T");
    test_equals(find_type_of_get_index(list5)->name, "number");
}

void compare_builtin_types()
{
    Branch branch;
    Term* aFloat = branch.compile("1.2");
    test_assert(term_output_always_satisfies_type(aFloat, unbox_type(FLOAT_TYPE)));
    test_assert(term_output_never_satisfies_type(aFloat, unbox_type(INT_TYPE)));

    Term* anInt = branch.compile("1");
    test_assert(term_output_always_satisfies_type(anInt, unbox_type(FLOAT_TYPE)));
    test_assert(term_output_always_satisfies_type(anInt, unbox_type(INT_TYPE)));
}

void for_loop_output_type()
{
    Branch branch;
    branch.compile("a = 1");
    branch.compile("for i in [1] { a = 2 }");

    test_assert(branch["a"]->type == INT_TYPE);
}

void assign_output_type()
{
    Branch branch;
    branch.compile("a = [1]");
    Term* assign = branch.compile("a[0] = 2");
    test_assert(assign->type == LIST_TYPE);
}

void infer_length()
{
    Branch branch;

    TaggedValue result;

    statically_infer_result(branch.compile("length([])"), &result);
    test_equals(&result, "0");
    statically_infer_result(branch.compile("length([1])"), &result);
    test_equals(&result, "1");
    statically_infer_result(branch.compile("length([1 2 3])"), &result);
    test_equals(&result, "3");
    statically_infer_result(branch.compile("length([1 2 3].append(3))"), &result);
    test_equals(&result, "4");
}

void register_tests()
{
    REGISTER_TEST_CASE(type_inference_tests::test_find_common_type);
    REGISTER_TEST_CASE(type_inference_tests::test_find_type_of_get_index);
    REGISTER_TEST_CASE(type_inference_tests::compare_builtin_types);
    REGISTER_TEST_CASE(type_inference_tests::for_loop_output_type);
    REGISTER_TEST_CASE(type_inference_tests::assign_output_type);
    REGISTER_TEST_CASE(type_inference_tests::infer_length);
}

} // namespace type_inference_tests
} // namespace circa
