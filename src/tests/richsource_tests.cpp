// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <circa.h>

namespace circa {
namespace richsource_tests {

void starter_test()
{
    Branch branch;
    branch.compile("a = 1");
    StyledSource source;
    format_branch_source(&source, &branch);
    //std::cout << source.toString() << std::endl;
}

void register_tests()
{
    REGISTER_TEST_CASE(richsource_tests::starter_test);
}

}
}
