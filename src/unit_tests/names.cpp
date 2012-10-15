// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "building.h"
#include "names.h"

namespace names {

void find_name()
{
    Branch branch;
    branch.compile("a = 1");
    branch.compile("namespace ns { a = 2; b = 3; } ");

    test_equals(term_value(find_name(&branch, "a")), "1");
    test_equals(term_value(find_name(&branch, "ns:a")), "2");
    test_equals(term_value(find_name(&branch, "ns:b")), "3");
}

void unique_ordinals()
{
    Branch branch;
    Term* a = branch.compile("a = 1");
    Term* b = branch.compile("b = 1");

    test_equals(a->uniqueOrdinal, 0);
    test_equals(b->uniqueOrdinal, 0);

    Term* a2 = branch.compile("a = 3");
    test_equals(a2->uniqueOrdinal, 1);

    Term* a3 = branch.compile("a = 3");
    test_equals(a3->uniqueOrdinal, 2);

    Term* x = branch.compile("x = 4");
    test_equals(x->uniqueOrdinal, 0);
    rename(x, name_from_string("a"));
    test_equals(x->uniqueOrdinal, 3);
}

void register_tests()
{
    REGISTER_TEST_CASE(names::find_name);
    REGISTER_TEST_CASE(names::unique_ordinals);
}

} // namespace names
