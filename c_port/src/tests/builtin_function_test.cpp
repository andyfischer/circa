
#include "common_headers.h"

#include <tests/common.h>
#include "branch.h"
#include "builtins.h"
#include "errors.h"
#include "operations.h"

void test_math()
{
    Branch* branch = new Branch();

    Term* two = constant_int(branch, 2);
    Term* three = constant_int(branch, 3);
    Term* negative_one = constant_int(branch, -1);
    Term* add_f = get_global("add");

    test_assert(as_int(apply_function(branch, add_f, TermList(two,three))) == 5);
    test_assert(as_int(apply_function(branch, add_f, TermList(two,negative_one))) == 1);
}

void builtin_functions_test()
{
    test_math();
}
