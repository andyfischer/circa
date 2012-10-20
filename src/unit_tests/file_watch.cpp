// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "fakefs.h"
#include "file_watch.h"
#include "kernel.h"
#include "world.h"

namespace file_watch {

void test_simple()
{
    World* world = global_world();
    FakeFilesystem files;

    files.set("file1", "x = 1");

    // Load the branch using a file watch.
    add_file_watch_module_load(world, "file1", "file_branch");
    file_watch_trigger_actions(world, "file1");

    test_equals(term_value(find_from_global_name(world, "file_branch:x")), "1");

    // Modify source file
    files.set("file1", "x = 2");

    // We haven't triggered the watch, so the old value should remain.
    test_equals(term_value(find_from_global_name(world, "file_branch:x")), "1");

    // Trigger watch, check that new value is loaded.
    file_watch_trigger_actions(world, "file1");
    test_equals(term_value(find_from_global_name(world, "file_branch:x")), "2");
}

void test_check_all_watches()
{
    World* world = global_world();
    FakeFilesystem files;

    files.set("file1", "x = 1");

    // Load the branch using a file watch.
    add_file_watch_module_load(world, "file1", "file_branch");
    file_watch_trigger_actions(world, "file1");

    test_equals(term_value(find_from_global_name(world, "file_branch:x")), "1");

    // Modify source file and mtime
    files.set("file1", "x = 2");
    files.set_mtime("file1", 1);

    // Check all file watches
    file_watch_check_all(world);

    // New script should be loaded.
    test_equals(term_value(find_from_global_name(world, "file_branch:x")), "2");

    // Modify source file without touching mtime
    files.set("file1", "x = 3");

    file_watch_check_all(world);

    // This change shouldn't be loaded.
    test_equals(term_value(find_from_global_name(world, "file_branch:x")), "2");

    // Now touch mtime, the latest change should be loaded.
    files.set_mtime("file1", 2);
    file_watch_check_all(world);
    test_equals(term_value(find_from_global_name(world, "file_branch:x")), "3");
}

void register_tests()
{
    REGISTER_TEST_CASE(file_watch::test_simple);
    REGISTER_TEST_CASE(file_watch::test_check_all_watches);
}

}
