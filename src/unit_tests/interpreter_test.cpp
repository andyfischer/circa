// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "block.h"
#include "hashtable.h"
#include "inspection.h"
#include "interpreter.h"
#include "kernel.h"
#include "function.h"
#include "importing.h"
#include "migration.h"
#include "modules.h"
#include "reflection.h"
#include "string_type.h"
#include "type.h"
#include "world.h"

namespace interpreter_test {

void test_cast_first_inputs()
{
    // Pass an input of [1] to a block that expects a compound type.
    // The function will need to cast the [1] to T in order for it to work.

    Block block;
    block.compile("type T { int i }");
    Term* f = block.compile("def f(T t) -> int { return t.i }");

    Stack stack;
    stack_init(&stack, function_contents(f));

    caValue* in = circa_input((caStack*) &stack, 0);
    circa_set_list(in, 1);
    circa_set_int(circa_index(in, 0), 5);

    run_interpreter(&stack);

    test_assert(circa_int(circa_output((caStack*) &stack, 0)) == 5);
}

void run_block_after_additions()
{
    Block block;

    // Create a block and run it.
    block.compile("a = 1");
    block.compile("test_spy(a)");
    block.compile("b = a + 2");
    block.compile("test_spy(b)");

    test_spy_clear();

    Stack stack;
    stack_init(&stack, &block);

    run_interpreter(&stack);

    test_equals(test_spy_get_results(), "[1, 3]");

    // Add some more stuff to the block, and run it. The Stack was not modified,
    // so it should continue where we stopped.
    block.compile("c = 4");
    block.compile("test_spy(c)");
    block.compile("d = a + b + c");
    block.compile("test_spy(d)");

    test_spy_clear();
    run_interpreter(&stack);

    test_equals(test_spy_get_results(), "[4, 8]");
}

void test_evaluate_minimum()
{
    // Test that rpath works in evaluate_minimum.
    test_write_fake_file("dir/block.ca", 1, "x = rpath('path'); y = concat(x, '/more_path')");

    Block* block = load_module_file(global_world(), "test_evaluate_minimum", "dir/block.ca");
    Term* y = find_local_name(block, "y");
    test_assert(y != NULL);

    Value value;
    evaluate_minimum2(y, &value);

    test_equals(&value, "dir/path/more_path");
}

void my_func_override(caStack* stack)
{
    set_int(circa_output(stack, 0), circa_int_input(stack, 0) + 10);
}

void test_directly_call_native_override()
{
    // Test an interpreter session where the top frame is a native override.
    
    Block block;
    Term* my_func = block.compile("def my_func(int i) -> int");
    install_function(&block, "my_func", my_func_override);

    Stack stack;
    stack_init(&stack, function_contents(my_func));

    set_int(circa_input(&stack, 0), 5);
    run_interpreter(&stack);
    test_equals(circa_output(&stack, 0), "15");
}

void bug_stale_bytecode_after_migrate()
{
    // There was a bug where Stack was holding on to stale bytecode, which caused
    // problems when the Block was migrated.

    Block version1;
    version1.compile("test_spy(1)");

    Block version2;
    version2.compile("test_spy(2)");

    Stack stack;
    stack_init(&stack, &version1);

    test_spy_clear();
    run_interpreter(&stack);
    test_equals(test_spy_get_results(), "[1]");

    stack_restart(&stack);
    migrate_stack(&stack, &version1, &version2);
    test_spy_clear();
    run_interpreter(&stack);
    test_equals(test_spy_get_results(), "[2]");
}

void bug_restart_dies_after_code_delete()
{
    Block version1;
    version1.compile("1 + 2");
    version1.compile("3 + 4");
    version1.compile("5 + 6");
    version1.compile("assert(false)");

    Block version2;
    version1.compile("1");

    Stack stack;
    stack_init(&stack, &version1);
    run_interpreter(&stack);

    migrate_stack(&stack, &version1, &version2);

    // This was causing a crash, internal NULL deref.
    stack_restart(&stack);
}

void test_inject_context()
{
    Block block;
    block.compile("test_spy(context(:a) + 5)");

    Stack stack;
    stack_init(&stack, &block);

    set_int(circa_inject_context(&stack, "a"), 5);
    test_spy_clear();
    run_interpreter(&stack);

    test_equals(test_spy_get_results(), "[10]");
}

Frame* g_retainFrameSaved = NULL;
bool g_retainFrameCalledStep2 = false;

void retain_frame_test_thunk_1(caStack* stack)
{
    g_retainFrameSaved = stack_top(stack);
    frame_retain(stack_top(stack));
}

void retain_frame_test_thunk_2(caStack* stack)
{
    g_retainFrameCalledStep2 = true;
    test_assert(stack_top(stack) == g_retainFrameSaved);
}

void test_retain_frame()
{
    g_retainFrameSaved = NULL;
    g_retainFrameCalledStep2 = false;

    Block block;
    block.compile("def thunk()");
    block.compile("def f() { thunk() }");
    block.compile("f()");

    install_function(&block, "thunk", retain_frame_test_thunk_1);

    Stack stack;
    stack_init(&stack, &block);

    run_interpreter(&stack);

    ca_assert(g_retainFrameSaved != NULL);

    install_function(&block, "thunk", retain_frame_test_thunk_2);

    stack_restart(&stack);
    run_interpreter(&stack);

    test_assert(g_retainFrameCalledStep2);

    stack_restart(&stack);
    run_interpreter(&stack);
}

void test_that_stack_is_implicitly_restarted_in_run_interpreter()
{
    Block block;
    compile(&block, "test_spy(1)");

    test_spy_clear();

    Stack stack;
    stack_init(&stack, &block);

    run_interpreter(&stack);

    test_equals(test_spy_get_results(), "[1]");

    run_interpreter(&stack);
    run_interpreter(&stack);
    run_interpreter(&stack);

    test_equals(test_spy_get_results(), "[1, 1, 1, 1]");
}

void register_tests()
{
    REGISTER_TEST_CASE(interpreter_test::test_cast_first_inputs);
    //REGISTER_TEST_CASE(interpreter_test::run_block_after_additions);
    //REGISTER_TEST_CASE(interpreter_test::test_evaluate_minimum);
    REGISTER_TEST_CASE(interpreter_test::test_directly_call_native_override);
    REGISTER_TEST_CASE(interpreter_test::bug_stale_bytecode_after_migrate);
    REGISTER_TEST_CASE(interpreter_test::bug_restart_dies_after_code_delete);
    REGISTER_TEST_CASE(interpreter_test::test_inject_context);
    REGISTER_TEST_CASE(interpreter_test::test_retain_frame);
    REGISTER_TEST_CASE(interpreter_test::test_that_stack_is_implicitly_restarted_in_run_interpreter);
}

} // namespace interpreter_test
