// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include <circa.h>

namespace circa {
namespace stateful_code_tests {

void test_load_and_save()
{
    Branch branch;
    Term* statefulTerm = branch.eval("state i = 1");
    Term* value = create_value(NULL, BRANCH_TYPE);
    Term* value_i = create_value(&as_branch(value), INT_TYPE, "i");
    as_int(value_i) = 5;

    test_assert(as_int(statefulTerm) == 1);

    load_state_into_branch(value, branch);

    test_assert(as_int(statefulTerm) == 5);

    as_int(statefulTerm) = 11;

    persist_state_from_branch(branch, value);

    test_assert(as_int(value_i) == 11);
}

void register_tests()
{
    REGISTER_TEST_CASE(stateful_code_tests::test_load_and_save);
}

} // namespace stateful_code_tests

} // namespace circa
