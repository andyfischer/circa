
#include "common_headers.h"

#include "bootstrapping.h"
#include "branch.h"
#include "builtins.h"
#include "common.h"
#include "errors.h"
#include "operations.h"
#include "subroutine.h"

namespace circa {

void test_simple()
{
    Branch *branch = new Branch();
    Term* my_sub = quick_create_subroutine(branch, "my-sub",
        TermList(get_global("int"), get_global("int")), get_global("int"));
    Subroutine* sub = as_subroutine(my_sub);

    // Try to create this formula:
    // result = a*2 + b*5
    Term* zero = constant_int(branch, 0);
    Term* one = constant_int(branch, 1);
    Term* two = constant_int(branch, 2);
    Term* five = constant_int(branch, 5);
    Term* a_times_two = apply_function(sub->branch, get_global("mult"),
        TermList(sub->inputPlaceholders[0], two));
    Term* b_times_five = apply_function(sub->branch, get_global("mult"),
        TermList(sub->inputPlaceholders[1], five));
    sub->outputPlaceholder = apply_function(sub->branch, get_global("add"),
        TermList(a_times_two, b_times_five));

    test_assert(as_int(exec_function(branch, my_sub, TermList(one, one))) == 7);
    test_assert(as_int(exec_function(branch, my_sub, TermList(one, two))) == 12);
    test_assert(as_int(exec_function(branch, my_sub, TermList(two, zero))) == 4);
	
	branch->bindName(one, "one");
	branch->bindName(two, "two");
	test_assert(as_int(quick_exec_function(branch, "my-sub(one,two)")) == 12);
}

void subroutine_test()
{
    test_simple();
}

}
