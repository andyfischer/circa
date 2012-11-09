// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "branch.h"
#include "evaluation.h"
#include "fakefs.h"
#include "kernel.h"
#include "modules.h"
#include "world.h"

namespace modules {

void source_file_location()
{
    FakeFilesystem fs;

    fs.set("branch.ca", "a = 1");
    Branch* branch = load_module_file(global_world(), "source_file_location", "branch.ca");

    test_equals(branch_get_source_filename(branch), "branch.ca");
}

void test_require()
{
    FakeFilesystem fs;

    fs.set("module.ca", "def f()->int { 5 }");

    fs.set("user.ca", "require module; test_spy(module:f())");

    Branch* branch = load_module_file(global_world(), "test_require", "user.ca");

    dump(branch);

    test_spy_clear();

    Stack stack;
    push_frame(&stack, branch);
    run_interpreter(&stack);

    test_assert(&stack);
    test_equals(test_spy_get_results(), "[5]");
}

void register_tests()
{
    REGISTER_TEST_CASE(modules::source_file_location);
    //REGISTER_TEST_CASE(modules::test_require);
}

} // namespace modules
