// Copyright 2008 Andrew Fischer

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

void register_tests()
{
    REGISTER_TEST_CASE(refactoring_tests::repro_source_after_rename);
}

} // namespace refactoring_tests

} // namespace circa
