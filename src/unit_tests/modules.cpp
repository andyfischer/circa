// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "branch.h"
#include "fakefs.h"
#include "kernel.h"
#include "world.h"

namespace modules {

void source_file_location()
{
    FakeFilesystem fs;

    fs.set("branch.ca", "a = 1");
    Branch* branch = load_script_to_global_name(global_world(), "branch.ca", "source_file_location");

    test_equals(branch_get_source_filename(branch), "branch.ca");
}

void register_tests()
{
    REGISTER_TEST_CASE(modules::source_file_location);
}

} // namespace modules
