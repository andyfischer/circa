// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "circa/file.h"

#include "fakefs.h"

namespace fakefs_test {

void test_simple()
{
    FakeFilesystem files;

    files.set("a", "hello");
    files.set("b", "goodbye");

    Value contents;

    circa_read_file("a", &contents);
    test_equals(&contents, "hello");

    circa_read_file("b", &contents);
    test_equals(&contents, "goodbye");

    files.set("a", "hello again");

    circa_read_file("a", &contents);
    test_equals(&contents, "hello again");
}

void register_tests()
{
    REGISTER_TEST_CASE(fakefs_test::test_simple);
}

} // namespace fakefs_test
