// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace list_tests {

void test_equals_branch()
{
    Branch branch;

    Term* a = create_branch(branch).owningTerm;
    create_int(as_branch(a), 4);
    create_string(as_branch(a), "hello");

    Term* b = branch.eval("[4 'hello']");
    Term* c = branch.eval("[4 'bye']");
    Term* d = branch.eval("[4]");
    Term* e = branch.eval("[]");

    test_assert(equals(a,b));
    test_assert(equals(b,a));
    test_assert(!equals(a,c));
    test_assert(!equals(a,d));
    test_assert(!equals(a,e));
}

void test_tagged_value()
{
    Branch branch;

    branch.eval("type MyType { string s, int i }");
    Term* val = branch.eval("MyType()");

    test_assert(is_string(val->getIndex(0)));
    test_assert(is_int(val->getIndex(1)));

    test_assert(is_string(val->getField("s")));
    test_assert(is_int(val->getField("i")));
    test_assert(val->getField("x") == NULL);
}

void register_tests()
{
    REGISTER_TEST_CASE(list_tests::test_equals_branch);
    REGISTER_TEST_CASE(list_tests::test_tagged_value);
}

}
}
