// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "circa.h"
#include "common.h"
#include "runtime.h"

namespace circa {
namespace subroutine_tests {

void create()
{
    Branch branch;

    Term* sub = eval_statement(branch, "sub = subroutine-create('my-sub, list(int), string)");


    test_assert(is_function(sub));
    test_assert(as_function(sub).name == "my-sub");
    test_assert(as_function(sub).inputTypes[0] == INT_TYPE);
    test_assert(as_function(sub).inputTypes.count() == 1);
    test_assert(as_function(sub).outputType == STRING_TYPE);

    // name input
    sub = eval_statement(branch, "function-name-input(@sub, 0, 'apple)");

    test_assert(is_function(sub));
    test_assert(as_function(sub).name == "my-sub");
    test_assert(as_function(sub).inputTypes[0] == INT_TYPE);
    test_assert(as_function(sub).inputTypes.count() == 1);
    test_assert(as_function(sub).outputType == STRING_TYPE);
}

void using_apply()
{
    Branch branch;

    eval_statement(branch, "sub = subroutine-create('s, list(int), float)");
    eval_statement(branch, "function-name-input(@sub, 0, 'x)");
    eval_statement(branch, "subroutine-apply(@sub, \"return add(mult(x,2)5)\"");
}

} // namespace subroutine_tests

void register_subroutine_tests()
{
    REGISTER_TEST_CASE(subroutine_tests::create);
    //REGISTER_TEST_CASE(subroutine_tests::using_apply);
}

} // namespace circa
