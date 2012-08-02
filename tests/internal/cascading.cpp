// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "framework.h"

#include "term.h"
#include "building.h"
#include "branch.h"

using namespace circa;

namespace cascading {

void test_rename()
{
    Branch branch;
    branch.compile("a = 1; b = 2; c = 3; d = add(b b)");

    Term* b = branch["b"];
    Term* c = branch["c"];
    Term* d = branch["d"];

    // sanity check: d starts with b as input.
    test_assert(d->input(0) == b);
    test_assert(d->input(1) == b);

    rename(c, name_from_string("b"));

    test_assert(d->input(0) == c);
    test_assert(d->input(1) == c);
}

} // namespace cascading

void cascading_register_tests()
{
    REGISTER_TEST_CASE(cascading::test_rename);
}
