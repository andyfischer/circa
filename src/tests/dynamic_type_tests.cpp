// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

namespace circa {
namespace dynamic_type_tests {

void test_copy()
{
    Branch branch;
    Term* a = branch.compile("a = 5");
    Term* copy = branch.compile("copy(a)");
    evaluate_branch(branch);

    test_assert(is_int(copy));

    change_type(a, STRING_TYPE);
    set_str(a, "hi");
    evaluate_branch(branch);
    test_assert(is_string(copy));
}

void test_subroutine()
{
    Branch branch;
    branch.compile("def f(any v) -> any return v end");
    test_assert(branch);

    Term* a = branch.eval("f('test')");

    test_assert(is_string(a));
    test_assert(as_string(a) == "test");

    branch.clear();
    branch.compile("def f(bool b, any v) -> any; if b return v else return 5 end end");

    Term* b = branch.eval("f(true, 'test')");
    test_assert(is_string(b));
    test_assert(as_string(b) == "test");
    Term* c = branch.eval("f(false, 'test')");
    test_assert(is_int(c));
    test_assert(as_int(c) == 5);
}

void register_tests()
{
    test_copy();
    test_subroutine();
}

} // namespace dynamic_type_tests
} // namespace circa
