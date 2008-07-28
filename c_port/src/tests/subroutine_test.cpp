
#include "common_headers.h"

#include "bootstrapping.h"
#include "branch.h"
#include "builtins.h"
#include "common.h"
#include "errors.h"
#include "operations.h"
#include "subroutine.h"

void test_simple()
{
    Branch *branch = new Branch();
    Term* my_sub = quick_create_subroutine(branch, "my-sub",
        TermList(get_global("int"), get_global("int")), get_global("int"));
    Subroutine* sub = as_subroutine(my_sub);

    // Try to create this formula:
    // result = a*2 + b*5
    Term* two = constant_int(branch, 2);
    Term* five = constant_int(branch, 5);
    Term* a_times_two = apply_function(sub->branch, get_global("mult"),
        TermList(sub->inputPlaceholders[0], two));
    Term* b_times_five = apply_function(sub->branch, get_global("mult"),
        TermList(sub->inputPlaceholders[0], five));
    sub->outputPlaceholder = apply_function(sub->branch, get_global("add"),
        TermList(a_times_two, b_times_five));
}

void subroutine_test()
{
    test_simple();
}
