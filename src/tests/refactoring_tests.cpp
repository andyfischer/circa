// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>
#include "importing_macros.h"

namespace circa {
namespace refactoring_tests {

CA_FUNCTION(_empty_evaluate) {}

void repro_source_after_rename()
{
    Branch branch;

    // Simple test
    Term* a = branch.compile("a = 1");

    rename(a, "apple");

    test_equals(get_term_source_text(a), "apple = 1");

    // Rename a function argument
    Term* b = branch.compile("b = 5");
    Term* add = branch.compile("add( b , 10)");

    rename(b, "banana");

    test_equals(get_term_source_text(add), "add( banana , 10)");

    // Rename a function
    Term* myfunc = import_function(branch, _empty_evaluate, "def myfunc(int)");
    Term* myfunc_call = branch.compile("myfunc(555)");

    rename(myfunc, "my_renamed_func");

    test_equals(get_term_source_text(myfunc_call), "my_renamed_func(555)");
}

void test_change_function()
{
    Branch branch;

    // simple test
    Term* a = branch.compile("add(3,3)");
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

    Term* target = branch.compile("target = {}");

    branch.eval("bm = branch_ref(target)");

    branch.eval("bm.append_code({ 1  +  1 })");

    test_equals(get_branch_source_text(target->nestedContents), " 1  +  1 ");
}

void register_tests()
{
    REGISTER_TEST_CASE(refactoring_tests::repro_source_after_rename);
    REGISTER_TEST_CASE(refactoring_tests::test_change_function);
    //TEST_DISABLED REGISTER_TEST_CASE(refactoring_tests::repro_source_after_append_code);
}

} // namespace refactoring_tests

} // namespace circa
