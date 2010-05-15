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

void register_tests()
{
    REGISTER_TEST_CASE(list_tests::test_equals_branch);
}

}
}
