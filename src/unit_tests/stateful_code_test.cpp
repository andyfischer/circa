// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "building.h"
#include "inspection.h"
#include "term.h"

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

void find_open_state_result_for_nested_return()
{
#if 0
    Block block;

    block.compile("def f() { state s; s = 1; if true { return } s = 2; }");

    Term* stateOutput = find_state_output(&block);
    Term* returnCall = find_term_from_path_expression(&block, "**/function=return");
    test_assert(returnCall != NULL);

    block.dump();
    Term* openState = find_open_state_result(returnCall);
    test_assert(openState != NULL);
    test_equals(term_value(openState), "1");

    Term* intermediateState = find_intermediate_result_for_output(returnCall, stateOutput);
    test_assert(intermediateState != NULL);
#endif
}

void register_tests()
{
    REGISTER_TEST_CASE(stateful_code_test::state_output_routing);
    REGISTER_TEST_CASE(stateful_code_test::find_open_state_result_for_nested_return);
}

}
