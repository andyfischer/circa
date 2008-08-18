
#include "common_headers.h"

#include "circa.h"
#include "common.h"

namespace circa {

void test_simple()
{
    Branch* branch = new Branch();

    Term* print_term = quick_exec_function(branch,
        "subroutine-create('print-term,list(string),void)");

    Term* input_names = constant_list(branch, TermList(constant_string(branch, "t")));
    print_term = exec_function(branch, get_global("subroutine-name-inputs"),
            TermList(print_term, input_names));
    branch->bindName(print_term, "print-term");

    quick_eval_function(as_subroutine(print_term)->branch, "print(to-string(t))");

    quick_exec_function(branch, "print-term('test)");
}

void test_using_subroutine_append()
{
    Branch* branch = new Branch();
    
    quick_exec_function(branch, "test-sub = subroutine-create('test-sub, list(int,int), int)");

    quick_exec_function(branch, "test-sub = subroutine-name-inputs(test-sub,list('a,'b))");
    quick_exec_function(branch, "a = subroutine-get-local(test-sub,'a)");
    quick_exec_function(branch, "b = subroutine-get-local(test-sub,'b)");

    quick_exec_function(branch, "print(subroutine-print(test-sub))");
    quick_exec_function(branch, "append-result = subroutine-append(test-sub, mult, list(a,4))");
    quick_exec_function(branch, "test-sub = get-field(append-result, 'sub)");
    quick_exec_function(branch, "a-times-4 = get-field(append-result, 'term)");

    quick_exec_function(branch, "append-result = subroutine-append(test-sub, mult, list(b,3))");
    quick_exec_function(branch, "test-sub = get-field(append-result, 'sub)");
    quick_exec_function(branch, "b-times-3 = get-field(append-result, 'term)");

    quick_exec_function(branch,
        "append-result = subroutine-append(test-sub, add, list(a-times-4,b-times-3))");
    quick_exec_function(branch, "test-sub = get-field(append-result, 'sub)");
    quick_exec_function(branch, "sum = get-field(append-result, 'term)");
    quick_exec_function(branch, "test-sub = subroutine-bind(test-sub, sum, 'output)");

    quick_exec_function(branch, "write-text-file('sub-append-test, export-graphviz(test-sub))");

    quick_exec_function(branch, "print(subroutine-print(test-sub))");

    // Finally, run it
    //std::cout << quick_exec_function(branch, "test-sub(2,3)")->toString();
}

void subroutine_test()
{
    // test_simple();
    test_using_subroutine_append();
}

}
