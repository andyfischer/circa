// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "testing.h"
#include "builtins.h"
#include "circa.h"
#include "importing.h"

namespace circa {
namespace importing_tests {

void my_imported_function(Term* term)
{
    as_int(term) = as_int(term->input(0)) + as_int(term->input(1));
}

void test_import_c()
{
    Branch branch;

    Term* func = import_function(branch, my_imported_function,
            "my_imported_func(int,int) : int");

    test_assert(function_get_output_type(func) == INT_TYPE);

    Term* result = branch.eval("my_imported_func(4,5)");

    test_assert(as_int(result) == 9);
}

void test_expose_value()
{
    Branch branch;

    int a = 1;

    expose_value(branch, &a, "a");
    Term* b = branch.eval("b = a + 2");

    test_assert(b);
    test_equals(to_float(b), 3);

    a = 10;

    evaluate_branch(branch);

    test_equals(to_float(b), 12);
}

void register_tests()
{
    REGISTER_TEST_CASE(importing_tests::test_import_c);
    REGISTER_TEST_CASE(importing_tests::test_expose_value);
}

} // namespace importing_tests

} // namespace circa
