// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "circa/file.h"

#include "fakefs.h"
#include "kernel.h"

namespace fakefs_test {

void test_simple()
{
    caWorld* world = global_world();
    FakeFilesystem files;

    files.set("a", "hello");
    files.set("b", "goodbye");

    Value contents;

    circa_read_file(world, "a", &contents);
    test_equals(&contents, "hello");

    circa_read_file(world, "b", &contents);
    test_equals(&contents, "goodbye");

    files.set("a", "hello again");

    circa_read_file(world, "a", &contents);
    test_equals(&contents, "hello again");
}

void register_tests()
{
#if 0 // TEST_DISABLED
    REGISTER_TEST_CASE(fakefs_test::test_simple);
#endif
}

} // namespace fakefs_test
