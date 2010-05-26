// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

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

void test_subroutine_input_and_output()
{
    Branch branch;

    branch.compile("def f(Point p); p.x; end");
    branch.compile("f([1 2])");

    evaluate_branch(branch);
    test_assert(branch);

    branch.compile("def f() -> any; return [1 1] -> Point end");
    branch.compile("a = f()");
    branch.compile("a.x");
    branch.compile("f().y");

    evaluate_branch(branch);
    test_assert(branch);
}

void test_dynamic_overload()
{
    Branch branch;
    Term* a = create_value(branch, ANY_TYPE, "a");
    Term* b = create_value(branch, ANY_TYPE, "b");
    Term* result = branch.compile("add(a, b)");

    make_int(a, 5);
    make_int(b, 3);

    Term* add_i = get_global("add_i");
    test_assert(add_i == overloaded_function::get_overload(ADD_FUNC, 0));

    test_assert(value_fits_type(a, type_contents(INT_TYPE)));

    RefList inputs(a,b);
    test_assert(inputs_fit_function_dynamic(add_i, inputs));

    evaluate_branch(branch);
    test_assert(result->asInt() == 8);

    make_float(b, 3.0);
    evaluate_branch(branch);
    test_assert(result->asFloat() == 8.0);
}

void register_tests()
{
    REGISTER_TEST_CASE(dynamic_type_tests::test_copy);
    REGISTER_TEST_CASE(dynamic_type_tests::test_subroutine);
    REGISTER_TEST_CASE(dynamic_type_tests::test_field_access);
    REGISTER_TEST_CASE(dynamic_type_tests::test_subroutine_input_and_output);
    REGISTER_TEST_CASE(dynamic_type_tests::test_dynamic_overload);
}

} // namespace dynamic_type_tests
} // namespace circa
