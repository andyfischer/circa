// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "tests/common.h"
#include "branch.h"
#include "builtins.h"
#include "runtime.h"
#include "term.h"
#include "values.h"

namespace circa {
namespace error_tests {

void test_evaluate()
{
    Branch branch;

    Term* one = float_var(branch, 1.0);

    Term* term1 = apply_function(branch, get_global("add"), ReferenceList(NULL, one));
    evaluate_term(term1);
    test_assert(term1->hasError());
    test_assert(term1->getErrorMessage() == "Input 0 is NULL");

    term1->function = NULL;
    evaluate_term(term1);
    test_assert(term1->hasError());
    test_assert(term1->getErrorMessage() == "Function is NULL");

    Term* term2 = apply_function(branch, get_global("add"), ReferenceList(one, NULL));
    evaluate_term(term2);
    test_assert(term2->hasError());
    test_assert(term2->getErrorMessage() == "Input 1 is NULL");

    set_input(term2, 1, one);
    evaluate_term(term2);
    test_assert(!term2->hasError());
    test_assert(term2->getErrorMessage() == "");
    test_assert(term2->asFloat() == 2.0);
}

} // namespace error_tests

void register_error_tests()
{
    REGISTER_TEST_CASE(error_tests::test_evaluate);
}

} // namespace circa
