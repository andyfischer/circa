// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "building.h"
#include "block.h"
#include "hashtable.h"
#include "inspection.h"
#include "interpreter.h"
#include "kernel.h"
#include "term.h"

namespace state_test {

void simple_declared_value()
{
    Block block;
    compile(&block, "state s");
    compile(&block, "s = 1");

    Stack stack;
    stack_init(&stack, &block);
    stack_run(&stack);

    test_assert(&stack);

    Value state;
    stack_extract_state(&stack, &state);
    test_equals(&state, "{s: 1}");
}

void simple_declared_value_2()
{
    Block block;
    compile(&block, "state s = 0");
    compile(&block, "s += 1");

    Stack stack;
    stack_init(&stack, &block);
    stack_run(&stack);

    test_assert(&stack);

    Value state;
    stack_extract_state(&stack, &state);
    test_equals(&state, "{s: 1}");

    stack_run(&stack);
    stack_extract_state(&stack, &state);
    test_equals(&state, "{s: 2}");

    stack_run(&stack);
    stack_extract_state(&stack, &state);
    test_equals(&state, "{s: 3}");
}

void function_state()
{
    Block block;
    compile(&block, "def f() { state s = 0; s += 1; test_spy(concat('s = ' s)) }");
    compile(&block, "call1 = f()");
    compile(&block, "call2 = f()");
    compile(&block, "call3 = f()");

    Stack stack;
    stack_init(&stack, &block);
    test_spy_clear();
    stack_run(&stack);
    test_assert(&stack);

    Value state;
    stack_extract_state(&stack, &state);
    test_equals(hashtable_get(&state, "call1"), "{s: 1}");
    test_equals(hashtable_get(&state, "call2"), "{s: 1}");
    test_equals(hashtable_get(&state, "call3"), "{s: 1}");
    test_equals(test_spy_get_results(), "['s = 1', 's = 1', 's = 1']");

    test_spy_clear();
    stack_run(&stack);
    test_assert(&stack);
    stack_extract_state(&stack, &state);
    test_equals(hashtable_get(&state, "call1"), "{s: 2}");
    test_equals(hashtable_get(&state, "call2"), "{s: 2}");
    test_equals(hashtable_get(&state, "call3"), "{s: 2}");
    test_equals(test_spy_get_results(), "['s = 2', 's = 2', 's = 2']");
}

void function_state_2()
{
    Block block;
    compile(&block, "def f() { state s = 0; s += 1; test_spy(concat('s = ' s)) }");
    compile(&block, "def g() { f() }");
    compile(&block, "def h() { g() }");
    compile(&block, "h()");

    // First call.
    Stack stack;
    stack_init(&stack, &block);
    test_spy_clear();
    stack_run(&stack);
    test_assert(&stack);

    Value state;
    stack_extract_state(&stack, &state);
    test_equals(&state, "{_h: {_g: {_f: {s: 1}}}}");
    test_equals(test_spy_get_results(), "['s = 1']");

    // Second call, with existing state.
    test_spy_clear();
    stack_run(&stack);
    test_assert(&stack);
    stack_extract_state(&stack, &state);
    test_equals(&state, "{_h: {_g: {_f: {s: 2}}}}");
    test_equals(test_spy_get_results(), "['s = 2']");
}

void if_block_state()
{
    Block block;
    compile(&block, "def f() { state s = 0; s += 1; test_spy(concat('s = ' s)) }");
    compile(&block, "if test_oracle() { left = f() } else { right = f() }");

    Stack stack;
    stack_init(&stack, &block);

    test_oracle_clear();
    set_bool(test_oracle_append(), true);
    test_spy_clear();
    stack_run(&stack);

    // First call on left side.
    Value state;
    stack_extract_state(&stack, &state);
    test_equals(&state, "{_if: [{left: {s: 1}}]}");
    test_equals(test_spy_get_results(), "['s = 1']");

    // Repeat left side.
    test_oracle_clear();
    set_bool(test_oracle_append(), true);
    test_spy_clear();
    stack_run(&stack);

    stack_extract_state(&stack, &state);
    test_equals(&state, "{_if: [{left: {s: 2}}]}");
    test_equals(test_spy_get_results(), "['s = 2']");

    // First call on right side.
    test_oracle_clear();
    set_bool(test_oracle_append(), false);
    test_spy_clear();
    stack_run(&stack);

    stack_extract_state(&stack, &state);
    test_equals(&state, "{_if: [null, {right: {s: 1}}]}");
    test_equals(test_spy_get_results(), "['s = 1']");
}

void for_loop_state_1()
{
    Block block;
    compile(&block, "for i in [0 1 2] { state s=0; s += 1; test_spy(concat('s = ' s)) }");

    Stack stack;
    stack_init(&stack, &block);

    // First call.
    test_spy_clear();
    stack_run(&stack);

    Value state;
    stack_extract_state(&stack, &state);
    test_equals(&state, "{_for: [{s: 1}, {s: 1}, {s: 1}]}");
    test_equals(test_spy_get_results(), "['s = 1', 's = 1', 's = 1']");

    // Second call, with existing state.
    test_spy_clear();
    stack_run(&stack);
    test_assert(&stack);
    stack_extract_state(&stack, &state);
    test_equals(&state, "{_for: [{s: 2}, {s: 2}, {s: 2}]}");
    test_equals(test_spy_get_results(), "['s = 2', 's = 2', 's = 2']");
}

void for_loop_state_2()
{
    Block block;
    compile(&block, "def f() { state s = 0; s += 1; test_spy(concat('s = ' s)) }");
    compile(&block, "for i in [0 1 2] { f() }");

    Stack stack;
    stack_init(&stack, &block);

    // First call.
    test_spy_clear();
    stack_run(&stack);

    Value state;
    stack_extract_state(&stack, &state);
    test_equals(&state, "{_for: [{_f: {s: 1}}, {_f: {s: 1}}, {_f: {s: 1}}]}");
    test_equals(test_spy_get_results(), "['s = 1', 's = 1', 's = 1']");

    // Second call, with existing state.
    test_spy_clear();
    stack_run(&stack);
    test_assert(&stack);
    stack_extract_state(&stack, &state);
    test_equals(&state, "{_for: [{_f: {s: 2}}, {_f: {s: 2}}, {_f: {s: 2}}]}");
    test_equals(test_spy_get_results(), "['s = 2', 's = 2', 's = 2']");
}

void register_tests()
{
    REGISTER_TEST_CASE(state_test::simple_declared_value);
    REGISTER_TEST_CASE(state_test::simple_declared_value_2);
    REGISTER_TEST_CASE(state_test::function_state);
    REGISTER_TEST_CASE(state_test::function_state_2);
    REGISTER_TEST_CASE(state_test::if_block_state);
    REGISTER_TEST_CASE(state_test::for_loop_state_1);
    REGISTER_TEST_CASE(state_test::for_loop_state_2);
}

} // namespace state_test
