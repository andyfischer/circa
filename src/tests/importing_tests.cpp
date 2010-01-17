// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include "testing.h"
#include "builtins.h"
#include "circa.h"
#include "importing.h"

namespace circa {
namespace importing_tests {

void my_imported_function(Term* term)
{
    set_int(term, as_int(term->input(0)) + as_int(term->input(1)));
}

void test_import_c()
{
    Branch branch;

    Term* func = import_function(branch, my_imported_function,
            "my_imported_func(int,int) -> int");

    test_assert(function_t::get_output_type(func) == INT_TYPE);

    Term* result = branch.eval("my_imported_func(4,5)");

    test_assert(as_int(result) == 9);
}

void register_tests()
{
    REGISTER_TEST_CASE(importing_tests::test_import_c);
}

} // namespace importing_tests

} // namespace circa
