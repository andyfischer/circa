// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "tests/common.h"
#include "builtins.h"
#include "circa.h"
#include "importing.h"

namespace circa {
namespace importing_tests {

void my_imported_function(Term* term)
{
    as_int(term) = as_int(term->inputs[0]) + as_int(term->inputs[1]);
}

void test_import_c()
{
    Branch branch;

    Term* func_as_term = import_c_function(branch, my_imported_function,
            "function my-imported-func(int,int) -> int");

    test_assert(as_function(func_as_term).outputType == INT_TYPE);
    test_assert(as_function(func_as_term).stateType == VOID_TYPE);

    Term* result = eval_statement(branch, "my-imported-func(4,5)");

    test_assert(as_int(result) == 9);
}

} // namespace importing_tests

void register_importing_tests()
{
    REGISTER_TEST_CASE(importing_tests::test_import_c);
}

} // namespace circa
