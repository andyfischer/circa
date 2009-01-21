
// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "circa.h"
#include "testing.h"

namespace circa {
namespace term_pointer_iterator_tests {

void simple()
{
    Branch branch;
    Term *one = branch.eval("one = 1.0");
    Term *two = branch.eval("two = 2.0");
    Term *t = branch.eval("a = add(one,two)");

    TermPointerIterator it(t);

    test_assert(it.current() == one);
    it.advance();
    test_assert(it.current() == two);
    it.advance();
    test_assert(it.current() == ADD_FUNC);
    it.advance();
    test_assert(it.current() == FLOAT_TYPE);
    it.advance();
    test_assert(it.finished());
}

}

void register_term_pointer_iterator_tests()
{
    REGISTER_TEST_CASE(term_pointer_iterator_tests::simple);
}

} // namespace circa
