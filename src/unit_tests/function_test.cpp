// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "block.h"
#include "building.h"
#include "closures.h"
#include "function.h"
#include "interpreter.h"
#include "kernel.h"

namespace function_test {
 
void access_function_as_value()
{
    Block block;
    Term* func = block.compile("def func()");

    Block block2;
    Term* call = block2.compile("test_spy()");
    set_input(call, 0, func);

    test_spy_clear();

    Stack stack;
    stack_init(&stack, &block2);
    run_interpreter(&stack);

    caValue* funcValue = list_get(test_spy_get_results(), 0);
    test_assert(is_closure(funcValue));
    test_assert(as_block(closure_get_block(funcValue)) == func->nestedContents);
}

void register_tests()
{
    REGISTER_TEST_CASE(function_test::access_function_as_value);
}

} // namespace function_test
