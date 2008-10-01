// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "circa.h"
#include "common.h"
#include "evaluation.h"

namespace circa {
namespace subroutine_tests {

    /*
void test_simple()
{
    Branch branch;

    Term* print_term = eval_statement(branch,
        "subroutine-create('print-term,list(string),void)");

    Term* input_names = constant_list(&branch, ReferenceList(constant_string(&branch, "t")));
    print_term = eval_function(&branch, get_global("subroutine-name-inputs"),
            ReferenceList(print_term, input_names));
    branch.bindName(print_term, "print-term");

    eval_statement(as_subroutine(print_term).branch, "print(to-string(t))");

    eval_statement(branch, "print-term('test)");
}

void test_using_subroutine_eval()
{
    Branch branch;

    eval_statement(branch, "sub = subroutine-create('test-sub, list(float,float), float)");
    eval_statement(branch, "sub = subroutine-name-inputs(sub, list('a,'b))");
    eval_statement(branch, "sub = subroutine-eval(sub, \"a-times-four = mult(a,4)\")");
    eval_statement(branch, "sub = subroutine-eval(sub, \"b-times-three = mult(b,3)\")");
    eval_statement(branch, "sub = subroutine-eval(sub, \"return add(a-times-four,b-times-three)\")");

    test_assert(int(as_float(eval_statement(branch, "sub(1,2))"))) == 10);
}

void test_using_evaluator()
{
    Branch branch;

    eval_statement(branch, "sub = subroutine-create('test-sub, list(float,float), float)");
    eval_statement(branch, "sub = subroutine-name-inputs(sub, list('a,'b))");
    eval_statement(branch, "sub = subroutine-eval(sub, \"a-times-four = mult(a,4.0)\")");
    eval_statement(branch, "sub = subroutine-eval(sub, \"b-times-three = mult(b,3.0)\")");
    eval_statement(branch, "sub = subroutine-eval(sub, \"return add(a-times-four,b-times-three)\")");
    
    evaluation::Engine evaluator;
    Term* result = eval_statement(branch, "sub(2.0,1.0)");
    evaluator.evaluate(result);
    evaluator.runUntilFinished();

    test_assert(as_float(result) == 11);
}
*/

void create()
{
    Branch branch;

    Term* sub = eval_statement(branch, "sub = subroutine-create('my-sub, list(int), string)");

    //test_assert(is_subroutine(sub));
    test_assert(as_function(sub).name == "my-sub");
    test_assert(as_function(sub).inputTypes[0] == INT_TYPE);
    test_assert(as_function(sub).inputTypes.count() == 1);
    test_assert(as_function(sub).outputType == STRING_TYPE);

    // name input
    sub = eval_statement(branch, "function-name-input(@sub, 0, 'apple)");
}

void using_apply()
{
    Branch branch;

    eval_statement(branch, "sub = subroutine-create('s, list(int), int)");
    eval_statement(branch, "function-name-input(@sub, 0, 'x)");
    eval_statement(branch, "subroutine-apply(@sub, \"return add(mult(x,2)5)\"");
}

} // namespace subroutine_tests

void register_subroutine_tests()
{
    //REGISTER_TEST_CASE(subroutine_tests::create);
    REGISTER_TEST_CASE(subroutine_tests::using_apply);
}

} // namespace circa
