// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "term.h"
#include "building.h"
#include "block.h"

namespace cascading {

void test_rename()
{
    Block block;
    block.compile("a = 1; b = 2; c = 3; d = add(b b)");

    Term* b = block["b"];
    Term* c = block["c"];
    Term* d = block["d"];

    // sanity check: d starts with b as input.
    test_assert(d->input(0) == b);
    test_assert(d->input(1) == b);

    rename(c, "b");

    test_assert(d->input(0) == c);
    test_assert(d->input(1) == c);
}

void register_tests()
{
    REGISTER_TEST_CASE(cascading::test_rename);
}

} // namespace cascading
