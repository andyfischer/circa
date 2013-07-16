// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "circa/file.h"

#include "filepack.h"
#include "kernel.h"

namespace fakefs_test {

void test_simple()
{
    caWorld* world = global_world();

    test_write_fake_file("a", 1, "hello");
    test_write_fake_file("b", 1, "goodbye");

    Value contents;

    circa_read_file(world, "a", &contents);
    test_equals(&contents, "hello");

    circa_read_file(world, "b", &contents);
    test_equals(&contents, "goodbye");

    test_write_fake_file("a", 1, "hello again");

    circa_read_file(world, "a", &contents);
    test_equals(&contents, "hello again");
}

void register_tests()
{
    REGISTER_TEST_CASE(fakefs_test::test_simple);
}

} // namespace fakefs_test
