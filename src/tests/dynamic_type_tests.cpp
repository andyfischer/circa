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

void test_field_access()
{
    Branch branch;

    Term* T = branch.compile("type T { int a, string b }");
    branch.compile("def f() -> any; return T([4, 's']); end");
    Term* r = branch.compile("r = f()");
    evaluate_branch(branch);
    branch.compile("r.a");

    test_assert(branch);
    test_assert(branch.eval("r.a == 4"));
    test_assert(branch.eval("r.b == 's'"));

    test_assert(r->type == ANY_TYPE);
    test_assert(r->value_type == get_pointer(T));

    branch.eval("r.b = 's2'");
    test_assert(branch);
}

void test_subroutine_inputs()
{
    Branch branch;

    branch.compile("def f(Point p); p.x; end");
    branch.compile("f([1 2])");

    evaluate_branch(branch);
    test_assert(branch);
}

void register_tests()
{
    REGISTER_TEST_CASE(dynamic_type_tests::test_copy);
    REGISTER_TEST_CASE(dynamic_type_tests::test_subroutine);
    REGISTER_TEST_CASE(dynamic_type_tests::test_field_access);
    //REGISTER_TEST_CASE(dynamic_type_tests::test_subroutine_inputs);
}

} // namespace dynamic_type_tests
} // namespace circa
