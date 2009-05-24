// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "circa.h"
#include "testing.h"
#include "introspection.h"
#include "runtime.h"

namespace circa {
namespace function_tests {

void create()
{
    Branch branch;

    Term* sub = branch.eval("def mysub(int) : string\nend");

    test_assert(sub);
    test_assert(is_subroutine(sub));

    Function& func = get_subroutines_function_def(sub);

    test_assert(func.name == "mysub");

    // the inputs are [Branch int] because we assume that every
    // subroutine has hidden state
    test_assert(identity_equals(func.inputTypes[1], INT_TYPE));
    test_assert(func.inputTypes.count() == 2);
    test_assert(identity_equals(func.outputType, STRING_TYPE));
}

void register_tests()
{
    REGISTER_TEST_CASE(function_tests::create);
}

} // namespace function_tests

} // namespace circa
