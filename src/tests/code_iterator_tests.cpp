// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace code_iterator_tests {

void test_simple()
{
    Branch branch;

    Term* a = branch.eval("a = 1");
    Term* b = branch.eval("b = 2");

    CodeIterator it(&branch);

    test_assert(it.current() == a);
    it.advance();
    test_assert(it.current() == b);
    it.advance();
    test_assert(it.finished());

    Term* funcTerm = create_value(&branch, FUNCTION_TYPE);
    Function &func = as_function(funcTerm);

    Term* c = func.subroutineBranch.eval("c = 3");
    Term* d = func.subroutineBranch.eval("d = 4");

    Term* e = branch.eval("e = 5");

    it.reset(&branch);

    test_assert(it.current() == a);
    it.advance();
    test_assert(it.current() == b);
    it.advance();
    test_assert(it.current() == funcTerm);
    it.advance();
    test_assert(it.current() == c);
    it.advance();
    test_assert(it.current() == d);
    it.advance();
    test_assert(it.current() == e);
    it.advance();
    test_assert(it.finished());
}

void register_tests()
{
    REGISTER_TEST_CASE(code_iterator_tests::test_simple);
}

}
}
