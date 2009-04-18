// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include <circa.h>

namespace circa {
namespace introspection_tests {

void test_is_value()
{
    Branch branch;

    test_assert(is_value(create_value(&branch, INT_TYPE)));
    test_assert(is_value(create_value(&branch, STRING_TYPE)));
    test_assert(is_value(create_value(&branch, BOOL_TYPE)));
    test_assert(!is_value(branch.eval("1 + 2")));
}

void test_get_involved_terms()
{
    Branch branch;

    Term* a = branch.eval("a = 1.0");
    branch.eval("b = 2.0");
    Term* c = branch.eval("c = add(a,b)");
    Term* d = branch.eval("d = add(c,5.0)");
    branch.eval("e = add(c,5.0)");

    RefList subtree = get_involved_terms(RefList(a), RefList(d));

    test_equals(subtree, RefList(a,c,d));
}

void test_find_equivalent()
{
    Branch branch;

    Term* add_func = branch.eval("add");
    Term* a = branch.eval("a = 1.0");
    Term* b = branch.eval("b = 1.0");
    Term* addition = branch.eval("add(a,b)");

    test_assert(is_equivalent(addition, add_func, RefList(a,b)));

    test_assert(addition == find_equivalent(branch, add_func, RefList(a,b)));

    test_assert(NULL == find_equivalent(branch, add_func, RefList(b,a)));
}

void register_tests()
{
    REGISTER_TEST_CASE(introspection_tests::test_is_value);
    REGISTER_TEST_CASE(introspection_tests::test_get_involved_terms);
    REGISTER_TEST_CASE(introspection_tests::test_find_equivalent);
}

} // namespace introspection_tests

} // namespace circa
