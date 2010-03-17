// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace list_tests {

void test_simple()
{
    List list;
    test_assert(list.length() == 0);
    list.append();
    test_assert(list.length() == 1);
    list.append();
    test_assert(list.length() == 2);
    list.clear();
    test_assert(list.length() == 0);
}

void test_tagged_value()
{
    Type list;
    list_t::setup_type(&list);

    TaggedValue value;
    change_type(&value, &list);

    test_assert(to_string(&value) == "[]");

}

void register_tests()
{
    REGISTER_TEST_CASE(list_tests::test_simple);
    REGISTER_TEST_CASE(list_tests::test_tagged_value);
}

}
}
