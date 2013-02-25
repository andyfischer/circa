// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "block.h"
#include "hashtable.h"
#include "inspection.h"
#include "interpreter.h"
#include "kernel.h"
#include "fakefs.h"
#include "function.h"
#include "importing.h"
#include "modules.h"
#include "string_type.h"
#include "type.h"
#include "world.h"

namespace interpreter {

void test_cast_first_inputs()
{
    // Pass an input of [1] to a block that expects a compound type.
    // The function will need to cast the [1] to T in order for it to work.

    Block block;
    block.compile("type T { int i }");
    Term* f = block.compile("def f(T t) -> int { return t.i }");

    Stack stack;
    push_frame(&stack, function_contents(f));

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
    push_frame(&stack, &block);

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
    // Test that rpath works in evaluate minimum.

    FakeFilesystem fs;
    fs.set("dir/block.ca", "x = rpath('path'); y = concat(x, '/more_path')");

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
    push_frame(&stack, function_contents(my_func));

    set_int(circa_input(&stack, 0), 5);
    run_interpreter(&stack);
    test_equals(circa_output(&stack, 0), "15");
}

void register_tests()
{
    REGISTER_TEST_CASE(interpreter::test_cast_first_inputs);
    REGISTER_TEST_CASE(interpreter::run_block_after_additions);
    REGISTER_TEST_CASE(interpreter::test_evaluate_minimum);
    REGISTER_TEST_CASE(interpreter::test_directly_call_native_override);
}

} // namespace interpreter
