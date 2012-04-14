// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "testing.h"
#include "kernel.h"
#include "circa_internal.h"
#include "importing.h"
#include "importing_macros.h"

namespace circa {
namespace importing_tests {

CA_FUNCTION(my_imported_function)
{
    set_int(OUTPUT, as_int(INPUT(0)) + as_int(INPUT(1)));
}

void test_import_c()
{
    Branch branch;

    Term* func = import_function(&branch, my_imported_function,
            "my_imported_func(int,int) -> int");

    test_equals(function_get_output_type(func, 0)->name, "int");

    Term* result = branch.compile("my_imported_func(4,5)");
    evaluate_branch(&branch);

    test_assert(as_int(result) == 9);
}

void test_import_type()
{
    Type* type = create_type();
    Branch branch;

    type->name = "A";

    Term* term = import_type(&branch, type);
    test_assert(term->type->name == "Type");
    test_assert(term->name == "A");
}

void register_tests()
{
    REGISTER_TEST_CASE(importing_tests::test_import_c);
    REGISTER_TEST_CASE(importing_tests::test_import_type);
}

} // namespace importing_tests

} // namespace circa
