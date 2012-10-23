// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "file.h"
#include "modules.h"
#include "tagged_value.h"

namespace file {

void test_get_just_filename_for_path()
{
    Value val;
    Value filename;

    set_string(&val, "a/long/path.ca");
    get_just_filename_for_path(&val, &filename);
    test_equals(&filename, "path.ca");
}

void test_module_get_default_name_from_filename()
{
    Value val;
    Value filename;

    set_string(&val, "a/long/path.ca");
    module_get_default_name_from_filename(&val, &filename);
    test_equals(&filename, "path");

    set_string(&val, "path");
    module_get_default_name_from_filename(&val, &filename);
    test_equals(&filename, "path");

    set_string(&val, "my.website/path");
    module_get_default_name_from_filename(&val, &filename);
    test_equals(&filename, "path");
}

void register_tests()
{
    REGISTER_TEST_CASE(file::test_get_just_filename_for_path);
    REGISTER_TEST_CASE(file::test_module_get_default_name_from_filename);
}

}
