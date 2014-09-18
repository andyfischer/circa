// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "file.h"
#include "modules.h"
#include "tagged_value.h"

namespace file_test {

void test_get_just_filename_for_path()
{
    Value val;
    Value filename;

    set_string(&val, "a/long/path.ca");
    get_just_filename_for_path(&val, &filename);
    test_equals(&filename, "path.ca");
}

void test_get_parent_directory()
{
    Value path, result;

    set_string(&path, "a/b");
    get_parent_directory(&path, &result);
    test_equals(&result, "a");

    set_string(&path, "a/b/c");
    get_parent_directory(&path, &result);
    test_equals(&result, "a/b");

    set_string(&path, "a/b///c");
    get_parent_directory(&path, &result);
    test_equals(&result, "a/b");

    set_string(&path, "a/b/");
    get_parent_directory(&path, &result);
    test_equals(&result, "a");

    set_string(&path, "a");
    get_parent_directory(&path, &result);
    test_equals(&result, ".");

    set_string(&path, ".");
    get_parent_directory(&path, &result);
    test_equals(&result, ".");

    set_string(&path, "/a");
    get_parent_directory(&path, &result);
    test_equals(&result, "/");

    set_string(&path, "/");
    get_parent_directory(&path, &result);
    test_equals(&result, "/");
}

void register_tests()
{
    REGISTER_TEST_CASE(file_test::test_get_just_filename_for_path);
    REGISTER_TEST_CASE(file_test::test_get_parent_directory);
}

}
