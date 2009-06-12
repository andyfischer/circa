// Copyright 2008 Andrew Fischer

#include <circa.h>

namespace circa {
namespace branch_iterator_tests {

void test_simple()
{
    Branch branch;

    Term* a = branch.eval("a = 1");
    Term* b = branch.eval("b = 2");

    test_assert(a);
    test_assert(b);

    BranchIterator it(branch);

    test_assert(it.current() == a);
    it.advance();
    test_assert(it.current() == b);
    it.advance();
    test_assert(it.finished());

    Term* sub = branch.compile("def func()\nc = 3\nd = 4\nend");

    it.reset(branch);

    test_assert(it.current() == a);
    it.advance();
    test_assert(it.current() == b);
    it.advance();
    test_assert(it.current() == sub);
    it.advance();
    test_assert(it.current() == sub->asBranch()[0]);
    it.advance();
    test_assert(it.current() == sub->asBranch()[1]);
    it.advance();
    test_assert(it.current() == sub->asBranch()[2]);
    it.advance();
    test_assert(it.finished());
}

void register_tests()
{
    REGISTER_TEST_CASE(branch_iterator_tests::test_simple);
}

}
}
