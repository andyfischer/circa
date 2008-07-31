
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


    Term my_sub = quick_exec_function(branch,
        "my-sub = subroutine-create(\"my-sub\", create-list(int,int), int)");

    Subroutine* sub = as_subroutine(my_sub);

    // Try to create this formula:
    // result = a*2 + b*5
    Term zero = constant_int(branch, 0);
    Term one = constant_int(branch, 1);
    Term two = constant_int(branch, 2);
    Term five = constant_int(branch, 5);
    Term a_times_two = apply_function(sub->branch, get_global("mult"),
        TermList(sub->inputPlaceholders[0], two));
    Term b_times_five = apply_function(sub->branch, get_global("mult"),
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

void test_simple2()
{
    Branch* branch = new Branch();
    Term any_t = get_global("any");
    Term void_t = get_global("void");
    Term int_t = get_global("int");
    Term string_t = get_global("string");

    Term list_with_string = constant_list(branch, TermList(string_t));
    branch->bindName(list_with_string, "list-with-string_t");

    Term print_term = quick_exec_function(branch,
        "subroutine-create(\"print-term\",list-with-string_t,void)");

    //Term print_term = quick_create_subroutine(branch, "print-term", TermList(string_t), void_t);

    Term input_names = constant_list(branch, TermList(constant_string(branch, "t")));
    print_term = exec_function(branch, get_global("subroutine-name-inputs"),
            TermList(print_term, input_names));
    branch->bindName(print_term, "print-term");

    quick_eval_function(as_subroutine(print_term)->branch, "print(to-string(t))");

    quick_exec_function(branch, "print-term(\"test\")");
}

void subroutine_test()
{
    test_simple();
    test_simple2();
}

}
