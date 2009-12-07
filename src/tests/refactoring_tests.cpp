// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace refactoring_tests {

void _empty_evaluate(Term*) {}

void repro_source_after_rename()
{
    Branch branch;

    // Simple test
    Term* a = branch.eval("a = 1");

    rename(a, "apple");

    test_equals(get_term_source(a), "apple = 1");

    // Rename a function argument
    Term* b = branch.eval("b = 5");
    Term* add = branch.eval("add( b , 10)");

    rename(b, "banana");

    test_equals(get_term_source(add), "add( banana , 10)");

    // Rename a function
    Term* myfunc = import_function(branch, _empty_evaluate, "def myfunc(int)");
    Term* myfunc_call = branch.eval("myfunc(555)");

    rename(myfunc, "my_renamed_func");

    test_equals(get_term_source(myfunc_call), "my_renamed_func(555)");
}

void test_change_function()
{
    Branch branch;

    // simple test
    Term* a = branch.eval("add(3,3)");
    evaluate_branch(branch);
    test_assert(as_int(a) == 6);

    change_function(a, KERNEL->get("mult_i"));
    evaluate_branch(branch);
    test_assert(as_int(a) == 9);

    // Make sure that if we change an expression to value(), that its current
    // value is preserved.
    test_assert(function_t::get_output_type(VALUE_FUNC) == ANY_TYPE);
    change_function(a, VALUE_FUNC);
    test_assert(as_int(a) == 9);
    test_assert(a->numInputs() == 0); // Should truncate inputs
    evaluate_branch(branch);
    test_assert(as_int(a) == 9);
}

void repro_source_after_append_code()
{
    Branch branch;

    Term* target = branch.eval("target = {}");

    branch.eval("bm = branch_mirror(target)");

    branch.eval("bm.append_code({ 1  +  1 })");

    test_equals(get_branch_source(as_branch(target)), "1  +  1 ");
    //std::cout << get_branch_source(as_branch(target)) << std::endl;
    //dump_branch(as_branch(target));
}

void register_tests()
{
    REGISTER_TEST_CASE(refactoring_tests::repro_source_after_rename);
    REGISTER_TEST_CASE(refactoring_tests::test_change_function);
    REGISTER_TEST_CASE(refactoring_tests::repro_source_after_append_code);
}

} // namespace refactoring_tests

} // namespace circa
