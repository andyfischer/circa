// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace type_inference_tests {

void test_find_common_type()
{
    test_assert(find_common_type(RefList(INT_TYPE,INT_TYPE)) == INT_TYPE);
    test_assert(find_common_type(RefList(FLOAT_TYPE,FLOAT_TYPE)) == FLOAT_TYPE);
    test_assert(find_common_type(RefList(INT_TYPE,FLOAT_TYPE)) == FLOAT_TYPE);
    test_assert(find_common_type(RefList(BOOL_TYPE,STRING_TYPE)) == ANY_TYPE);
    test_assert(find_common_type(RefList(KERNEL->get("Point"),KERNEL->get("Rect"))) == LIST_TYPE);
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
}

void compare_builtin_types()
{
    Branch branch;
    Term* aFloat = branch.eval("1.2");
    test_assert(term_output_always_satisfies_type(aFloat, type_contents(FLOAT_TYPE)));
    test_assert(term_output_never_satisfies_type(aFloat, type_contents(INT_TYPE)));

    Term* anInt = branch.eval("1");
    test_assert(term_output_always_satisfies_type(anInt, type_contents(FLOAT_TYPE)));
    test_assert(term_output_always_satisfies_type(anInt, type_contents(INT_TYPE)));
}

void register_tests()
{
    REGISTER_TEST_CASE(type_inference_tests::test_find_common_type);
    REGISTER_TEST_CASE(type_inference_tests::test_find_type_of_get_index);
    REGISTER_TEST_CASE(type_inference_tests::compare_builtin_types);
}

}
}
