// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "block.h"
#include "inspection.h"
#include "interpreter.h"
#include "kernel.h"
#include "modules.h"
#include "names.h"
#include "world.h"

namespace modules_test {

#if 0
void source_file_location()
{
    test_write_fake_file("block.ca", 1, "a = 1");
    Block* block = load_module_file(global_world(),
        temp_string("source_file_location"), "block.ca");

    test_equals(block_get_source_filename(block), "block.ca");
}

void test_explicit_output()
{
    World* world = global_world();

    test_write_fake_file("module.ca", 1, "99 -> output");

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
    test_write_fake_file("module.ca", 1, "1");

    Block block;
    load_script(&block, "module.ca");

    test_assert(get_output_placeholder(&block, 0) != NULL);
    test_assert(get_output_placeholder(&block, 0)->input(0) == NULL);
}

void non_required_module_is_not_visible()
{
    World* world = global_world();

    test_write_fake_file("module_a.ca", 1, "a = 1");
    test_write_fake_file("module_b.ca", 1, "b = 1");
    Block* module_a = load_module_file(world, temp_string("module_a"), "module_a.ca");
    Block* module_b = load_module_file(world, temp_string("module_b"), "module_b.ca");
    test_assert(find_name(module_a, "a") != NULL);
    test_assert(find_name(module_b, "a") == NULL);
}
#endif

void register_tests()
{
#if 0
    REGISTER_TEST_CASE(modules_test::source_file_location);
    REGISTER_TEST_CASE(modules_test::test_explicit_output);
    REGISTER_TEST_CASE(modules_test::module_always_has_primary_output);
    REGISTER_TEST_CASE(modules_test::non_required_module_is_not_visible);
#endif
}

} // namespace modules_test
