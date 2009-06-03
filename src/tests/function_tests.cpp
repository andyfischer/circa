// Copyright 2008 Andrew Fischer

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

void test_is_callable()
{
    test_assert(is_callable(ADD_FUNC));

    Branch branch;
    Term* s = branch.eval("def mysub()\nend");
    test_assert(is_callable(s));

    Term* a = branch.eval("1");
    test_assert(!is_callable(a));
}

void overloaded_function()
{
    Branch branch;
    test_assert(find_named(&branch, "add") != NULL);

    // TODO
    return;

    Term* my_add = branch.eval("def add(string s, int i) : string\nend");

    test_assert(branch["add"]->type == OVERLOADED_FUNCTION_TYPE);
}

void register_tests()
{
    REGISTER_TEST_CASE(function_tests::create);
    REGISTER_TEST_CASE(function_tests::overloaded_function);
    REGISTER_TEST_CASE(function_tests::test_is_callable);
}

} // namespace function_tests

} // namespace circa
