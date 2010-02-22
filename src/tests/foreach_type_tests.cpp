// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace foreach_type_tests {

bool run_test_for_type(Type* type, List& exampleValues)
{
    // TODO
    return true;
}

void run()
{
    List intExamples;
    make_int(intExamples.append(), 0);
    make_int(intExamples.append(), -1);
    make_int(intExamples.append(), 5);
}

void register_tests()
{
    REGISTER_TEST_CASE(foreach_type_tests::run);
}

}
}
