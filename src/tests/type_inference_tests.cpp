// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <circa.h>

namespace circa {
namespace type_inference_tests {

void test_find_common_type()
{
    test_assert(find_common_type(TermList(INT_TYPE,INT_TYPE)) == INT_TYPE);
    test_assert(find_common_type(TermList(FLOAT_TYPE,FLOAT_TYPE)) == FLOAT_TYPE);
    test_assert(find_common_type(TermList(INT_TYPE,FLOAT_TYPE)) == FLOAT_TYPE);
    test_assert(find_common_type(TermList(BOOL_TYPE,STRING_TYPE)) == ANY_TYPE);
    test_assert(find_common_type(TermList(KERNEL->get("Point"),KERNEL->get("Rect"))) == LIST_TYPE);
}

void test_find_type_of_get_index()
{
    Branch branch;
    Term* range = branch.compile("range(4,5)");

    test_assert(find_type_of_get_index(range) == INT_TYPE);

    Term* list1 = branch.compile("['hello' 'hi' 'bye']");
    test_assert(find_type_of_get_index(list1) == STRING_TYPE);

    Term* list2 = branch.compile("[0 -1 11]");
    test_assert(find_type_of_get_index(list2) == INT_TYPE);

    Term* list3 = branch.compile("[]");
    test_assert(find_type_of_get_index(list3) == ANY_TYPE);

    Term* list4 = branch.compile("[0.1 4]");
    test_assert(find_type_of_get_index(list4) == FLOAT_TYPE);

    branch.compile("type T { number x, number y }");
    Term* list5 = branch.compile("[.1 .1] -> T");
    test_assert(find_type_of_get_index(list5) == FLOAT_TYPE);
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

void register_tests()
{
    REGISTER_TEST_CASE(type_inference_tests::test_find_common_type);
    REGISTER_TEST_CASE(type_inference_tests::test_find_type_of_get_index);
    REGISTER_TEST_CASE(type_inference_tests::compare_builtin_types);
    REGISTER_TEST_CASE(type_inference_tests::for_loop_output_type);
    REGISTER_TEST_CASE(type_inference_tests::assign_output_type);
}

} // namespace type_inference_tests
} // namespace circa
