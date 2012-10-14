// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "fakefs.h"
#include "kernel.h"

namespace file_watch {

void test_simple()
{
    FakeFilesystem files;

    World* world = global_world();

    files.set("file1", "x = 1");
    files.set_mtime("file1", 1);

    // questions:
    //  in practice what does the file_watch do?
    //   Does it have a global branch name, and it finds the branch that way?
    //   What about branches without a global name (should there be such a thing?)
    //    Maybe the unique name can always be used as a real name.
    //  how do we make a useful unit test for this?
}

void register_tests()
{
    REGISTER_TEST_CASE(file_watch::test_simple);
}

}
