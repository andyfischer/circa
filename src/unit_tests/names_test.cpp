// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "building.h"
#include "code_iterators.h"
#include "interpreter.h"
#include "inspection.h"
#include "kernel.h"
#include "modules.h"
#include "names.h"
#include "string_type.h"

namespace names_test {

void unique_ordinals()
{
    Block block;
    Term* a = block.compile("a = 1");
    Term* b = block.compile("b = 1");

    test_equals(a->uniqueOrdinal, 0);
    test_equals(b->uniqueOrdinal, 0);

    Term* a2 = block.compile("a = 3");
    test_equals(a->uniqueOrdinal, 1);
    test_equals(a2->uniqueOrdinal, 2);

    Term* a3 = block.compile("a = 3");
    test_equals(a3->uniqueOrdinal, 3);

    Term* x = block.compile("x = 4");
    test_equals(x->uniqueOrdinal, 0);
    rename(x, "a");
    test_equals(x->uniqueOrdinal, 4);
}

void test_find_ordinal_suffix()
{
    int endPos;

    endPos = 1;
    test_equals(name_find_ordinal_suffix("a", &endPos), -1);
    test_equals(endPos, 1);

    endPos = 3;
    test_equals(name_find_ordinal_suffix("a#2", &endPos), 2);
    test_equals(endPos, 1);

    endPos = 3;
    test_equals(name_find_ordinal_suffix("a##", &endPos), -1);
    test_equals(endPos, 3);

    endPos = 3;
    test_equals(name_find_ordinal_suffix("a12", &endPos), -1);
    test_equals(endPos, 3);

    endPos = 3;
    test_equals(name_find_ordinal_suffix("#12", &endPos), 12);
    test_equals(endPos, 0);

    endPos = 7;
    test_equals(name_find_ordinal_suffix("a#2:b#3", &endPos), 3);
    test_equals(endPos, 5);

    endPos = 3;
    test_equals(name_find_ordinal_suffix("a#2:b#3", &endPos), 2);
    test_equals(endPos, 1);
    
    // test using -1 as the end position
    endPos = -1;
    test_equals(name_find_ordinal_suffix("a#2", &endPos), 2);
    test_equals(endPos, 1);
}

void register_tests()
{
    REGISTER_TEST_CASE(names_test::unique_ordinals);
    REGISTER_TEST_CASE(names_test::test_find_ordinal_suffix);
}

} // namespace names
