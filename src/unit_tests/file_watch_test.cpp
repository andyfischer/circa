// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "file_watch.h"
#include "kernel.h"
#include "modules.h"
#include "world.h"

namespace file_watch_test {

void test_simple()
{
    World* world = global_world();

    test_write_fake_file("file1", 1, "x = 1");

    // Load the block using a file watch.
    add_file_watch_module_load(world, "file1", temp_string("file_block"));
    file_watch_trigger_actions(world, "file1");

    test_equals(term_value(find_from_global_name(world, "file_block:x")), "1");

    // Modify source file
    test_write_fake_file("file1", 2, "x = 2");

    // We haven't triggered the watch, so the old value should remain.
    test_equals(term_value(find_from_global_name(world, "file_block:x")), "1");

    // Trigger watch, check that new value is loaded.
    file_watch_trigger_actions(world, "file1");
    test_equals(term_value(find_from_global_name(world, "file_block:x")), "2");
}

void test_check_all_watches()
{
    World* world = global_world();

    test_write_fake_file("file1", 1, "x = 1");

    // Load the block using a file watch.
    add_file_watch_module_load(world, "file1", temp_string("file_block"));
    file_watch_trigger_actions(world, "file1");

    test_equals(term_value(find_from_global_name(world, "file_block:x")), "1");

    // Modify source file and version
    test_write_fake_file("file1", 2, "x = 2");

    // Check all file watches
    file_watch_check_all(world);

    // New script should be loaded.
    test_equals(term_value(find_from_global_name(world, "file_block:x")), "2");

    // Modify source file without touching version
    test_write_fake_file("file1", 2, "x = 3");

    file_watch_check_all(world);

    // This change shouldn't be loaded.
    test_equals(term_value(find_from_global_name(world, "file_block:x")), "2");

    // Now touch version, the latest change should be loaded.
    test_write_fake_file("file1", 3, "x = 3");
    file_watch_check_all(world);
    test_equals(term_value(find_from_global_name(world, "file_block:x")), "3");
}

void register_tests()
{
    REGISTER_TEST_CASE(file_watch_test::test_simple);
    REGISTER_TEST_CASE(file_watch_test::test_check_all_watches);
}

}
