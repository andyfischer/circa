// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "framework.h"

#include "branch.h"
#include "code_iterators.h"
#include "debug.h"

using namespace circa;

namespace code_iterators {

void branch_iterator_2()
{
    Branch branch;
    branch.compile("a = 1; b = 2; c = { d = 3; e = 4; } f = 5");

    BranchIterator2 it(&branch);
    test_assert(it.current()->name == "a"); ++it; test_assert(!it.finished());
    test_assert(it.current()->name == "b"); ++it; test_assert(!it.finished());
    test_assert(it.current()->name == "c"); ++it; test_assert(!it.finished());
    test_assert(it.current()->name == "d"); ++it; test_assert(!it.finished());
    test_assert(it.current()->name == "e"); ++it; test_assert(!it.finished());
    test_assert(it.current()->name == "f"); ++it; test_assert(it.finished());
}

void branch_iterator_2_2()
{
    Branch branch;
    branch.compile("a = { b = { c = { d = 3; e = 4; }}}");

    BranchIterator2 it(&branch);
    test_assert(it.current()->name == "a"); test_assert(!it.finished()); ++it;
    test_assert(it.current()->name == "b"); test_assert(!it.finished()); ++it;
    test_assert(it.current()->name == "c"); test_assert(!it.finished()); ++it;
    test_assert(it.current()->name == "d"); test_assert(!it.finished()); ++it;
    test_assert(it.current()->name == "e"); test_assert(!it.finished()); ++it;
    test_assert(it.finished());
}

void name_visible_iterator_1()
{
    Branch branch;
    branch.compile("a = 1");
    Term* b = branch.compile("b = 1");
    branch.compile("c = 1; d = 1; e = 1");
    branch.compile("b = 2; f = 1; g = 1");

    NameVisibleIterator it(b);
    test_equals(it.current()->name,"c"); test_assert(!it.finished()); ++it;
    test_equals(it.current()->name,"d"); test_assert(!it.finished()); ++it;
    test_equals(it.current()->name,"e"); test_assert(!it.finished()); ++it;
    test_assert(it.finished());
}

} // namespace code_iterators

void code_iterators_register_tests()
{
    REGISTER_TEST_CASE(code_iterators::branch_iterator_2);
    REGISTER_TEST_CASE(code_iterators::branch_iterator_2_2);
    REGISTER_TEST_CASE(code_iterators::name_visible_iterator_1);
}
