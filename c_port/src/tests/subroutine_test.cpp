
#include "common_headers.h"

#include "circa.h"
#include "common.h"

namespace circa {

void test_simple()
{
    Branch *branch = new Branch();

    // Try to create this formula:
    // result = a*2 + b*5

    Term* my_sub = quick_exec_function(branch,
        "my-sub = subroutine-create('my-sub, list(int,int), int)");
    my_sub = quick_exec_function(branch,
        "my-sub = subroutine-name-inputs(my-sub, list('a, 'b))");

    Subroutine* sub = as_subroutine(my_sub);

    Term* result = quick_exec_function(sub->branch,
        "output = add(mult(a,2),mult(b,5))");

    test_assert(as_int(exec_function(branch, my_sub, TermList(CONSTANT_1, CONSTANT_1))) == 7);
    test_assert(as_int(exec_function(branch, my_sub, TermList(CONSTANT_1, CONSTANT_2))) == 12);
    test_assert(as_int(exec_function(branch, my_sub, TermList(CONSTANT_2, CONSTANT_0))) == 4);
	
	test_assert(as_int(quick_exec_function(branch, "my-sub(1,2)")) == 12);
}

void test_simple2()
{
    Branch* branch = new Branch();
    Term* any_t = get_global("any");
    Term* void_t = get_global("void");
    Term* int_t = get_global("int");
    Term* string_t = get_global("string");

    Term* print_term = quick_exec_function(branch,
        "subroutine-create('print-term,list(string),void)");

    Term* input_names = constant_list(branch, TermList(constant_string(branch, "t")));
    print_term = exec_function(branch, get_global("subroutine-name-inputs"),
            TermList(print_term, input_names));
    branch->bindName(print_term, "print-term");

    quick_eval_function(as_subroutine(print_term)->branch, "print(to-string(t))");

    quick_exec_function(branch, "print-term(\"test\")");
}

void test_using_subroutine_append()
{
    Branch* branch = new Branch();
    
    quick_exec_function(branch, "test-sub = subroutine-create('test-sub, list(int,int), int)");
    quick_exec_function(branch, "test-sub = subroutine-name-inputs(test-sub,list('a,'b))");
    quick_exec_function(branch, "a = subroutine-get-local(test-sub,'a)");
    quick_exec_function(branch, "b = subroutine-get-local(test-sub,'b)");
    quick_exec_function(branch, "test-sub = subroutine-append(test-sub, add, list(a,b))");
}

void subroutine_test()
{
    test_simple();
    test_using_subroutine_append();
    //test_simple2();
}

}
