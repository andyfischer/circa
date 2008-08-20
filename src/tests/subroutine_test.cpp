// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "circa.h"
#include "common.h"
#include "evaluator.h"

namespace circa {
namespace subroutine_test {

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

void test_using_subroutine_eval()
{
    Branch* branch = new Branch();

    quick_exec_function(branch, "sub = subroutine-create('test-sub, list(int,int), int)");
    quick_exec_function(branch, "sub = subroutine-name-inputs(sub, list('a,'b))");
    quick_exec_function(branch, "sub = subroutine-eval(sub, \"a-times-four = mult(a,4)\")");
    quick_exec_function(branch, "sub = subroutine-eval(sub, \"b-times-three = mult(b,3)\")");
    quick_exec_function(branch, "sub = subroutine-eval(sub, \"return add(a-times-four,b-times-three)\")");

    test_assert(as_int(quick_exec_function(branch, "sub(1,2))")) == 10);
}

void test_using_evaluator()
{
    Branch* branch = new Branch();

    quick_exec_function(branch, "sub = subroutine-create('test-sub, list(int,int), int)");
    quick_exec_function(branch, "sub = subroutine-name-inputs(sub, list('a,'b))");
    quick_exec_function(branch, "sub = subroutine-eval(sub, \"a-times-four = mult(a,4)\")");
    quick_exec_function(branch, "sub = subroutine-eval(sub, \"b-times-three = mult(b,3)\")");
    quick_exec_function(branch, "sub = subroutine-eval(sub, \"return add(a-times-four,b-times-three)\")");
    
    Evaluator evaluator;
    Term* result = quick_eval_function(branch, "sub(2,1)");
    evaluator.evaluate(result);
    evaluator.runUntilFinished();

    std::cout << result->toString() << std::endl;

    test_assert(as_int(result) == 11);
}

void all_tests()
{
    test_using_subroutine_eval();
    test_using_evaluator();
}

} // namespace subroutine_test
} // namespace circa
