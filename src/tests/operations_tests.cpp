// Copyright 2008 Paul Hodge

#include "tests/common.h"
#include "branch.h"
#include "builtins.h"
#include "operations.h"
#include "parser.h"
#include "term.h"
#include "term_list.h"

namespace circa {
namespace operations_tests {

void safe_delete()
{
    Branch branch;

    Term* term1 = apply_statement(branch, "term1 = 1");
    Term* termSum = apply_statement(branch, "sum = add(term1,2)");

    test_assert(branch.containsName("term1"));
    test_assert(branch.containsName("sum"));
    test_assert(termSum->inputs[0] == term1);

    delete term1;

    test_assert(termSum->inputs[0] == NULL);
    test_assert(!branch.containsName("term1"));

    for (int i=0; i < branch.terms.count(); i++)
    {
        if (branch.terms[i] == term1)
            test_fail();
    }
}

void test_create_constant()
{
    Branch branch;
    Term *term = create_constant(&branch, INT_TYPE);

    test_assert(term->type == INT_TYPE);
}

void test_constant_int()
{
    Branch branch;
    Term *term = constant_int(&branch, -2);
    test_assert(as_int(term) == -2);

    Term *term2 = constant_int(&branch, 154, "george");
    test_assert(term2 == branch.getNamed("george"));
    test_assert(as_int(term2) == 154);
}

void test_misc()
{
    test_assert(is_type(TYPE_TYPE));
    test_assert(is_instance(TYPE_TYPE, TYPE_TYPE));

    test_assert(is_type(FUNCTION_TYPE));
    test_assert(is_instance(FUNCTION_TYPE, TYPE_TYPE));
}

} // namespace operations_tests

void register_operations_tests()
{
    REGISTER_TEST_CASE(operations_tests::safe_delete);
    REGISTER_TEST_CASE(operations_tests::test_create_constant);
    REGISTER_TEST_CASE(operations_tests::test_constant_int);
    REGISTER_TEST_CASE(operations_tests::test_misc);
}

} // namespace circa
