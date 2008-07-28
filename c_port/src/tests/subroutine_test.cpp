
#include "common_headers.h"

#include "bootstrapping.h"
#include "branch.h"
#include "builtins.h"
#include "common.h"
#include "errors.h"
#include "subroutine.h"

void test_simple()
{
    Branch *branch = new Branch();
    Term* my_sub = quick_create_subroutine(branch, "my-sub",
        TermList(get_global("int"), get_global("int")), get_global("int"));
}

void subroutine_test()
{
    test_simple();
}
