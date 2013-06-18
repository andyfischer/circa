// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "block.h"
#include "inspection.h"
#include "interpreter.h"
#include "fakefs.h"
#include "kernel.h"
#include "modules.h"
#include "world.h"

namespace modules_test {

void source_file_location()
{
    FakeFilesystem fs;

    fs.set("block.ca", "a = 1");
    Block* block = load_module_file(global_world(), "source_file_location", "block.ca");

    test_equals(block_get_source_filename(block), "block.ca");
}

void test_require()
{
    FakeFilesystem fs;

    fs.set("module.ca", "def f(String s)->int { test_spy(concat('f: ' s)) }");
    fs.set("user.ca", "require module\n"
      "f('without prefix')\n"
      "module:f('with prefix')"
      );

    load_module_file(global_world(), "module", "module.ca");
    Block* block = load_module_file(global_world(), "test_require", "user.ca");

    test_spy_clear();

    Stack stack;
    push_frame(&stack, block);
    run_interpreter(&stack);

    test_assert(&stack);
    test_equals(test_spy_get_results(), "['f: without prefix', 'f: with prefix']");
}

void test_explicit_output()
{
    World* world = global_world();

    FakeFilesystem fs;
    fs.set("module.ca", "99 -> output");

    circa_load_module_from_file(world, "Module", "module.ca");

    Stack* stack = circa_create_stack(world);

    circa_push_module(stack, "Module");
    circa_run(stack);

    test_assert(stack);
    test_equals(circa_output(stack, 0), "99");

    circa_free_stack(stack);
}

void module_always_has_primary_output()
{
    FakeFilesystem fs;
    fs.set("module.ca", "1");

    Block block;
    load_script(&block, "module.ca");

    test_assert(get_output_placeholder(&block, 0) != NULL);
    test_assert(get_output_placeholder(&block, 0)->input(0) == NULL);
}

void register_tests()
{
    REGISTER_TEST_CASE(modules_test::source_file_location);
    REGISTER_TEST_CASE(modules_test::test_require);
    REGISTER_TEST_CASE(modules_test::test_explicit_output);
    REGISTER_TEST_CASE(modules_test::module_always_has_primary_output);
}

} // namespace modules_test
