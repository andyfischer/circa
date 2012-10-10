// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "branch.h"
#include "type.h"

namespace branch {

void test_state_type()
{
    Branch branch;

    branch.compile("a = 1; b = 2; c = 3");
    branch_update_state_type(&branch);
    test_assert(branch.stateType == NULL);

    clear_branch(&branch);
    branch.compile("state a = 1; state b = 2.0");
    branch_update_state_type(&branch);
    test_assert(branch.stateType != NULL);

    test_equals(&branch.stateType->parameter, "[[<Type any>, <Type any>], ['a', 'b']]");
}

void register_tests()
{
    REGISTER_TEST_CASE(branch::test_state_type);
}

}
