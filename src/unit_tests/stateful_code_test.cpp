// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "building.h"
#include "inspection.h"

namespace stateful_code_test {

void state_output_routing()
{
    Block block;

    block.compile("state s");

    Term* stateOutput = find_state_output(&block);
    test_assert(stateOutput != NULL);

    Value description;
    get_output_description(stateOutput, &description);
    test_equals(&description, "[:State]");

    Term* t = find_output_from_description(&block, &description);
    test_assert(stateOutput == t);
}

void register_tests()
{
    REGISTER_TEST_CASE(stateful_code_test::state_output_routing);
}

}
