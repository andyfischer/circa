// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <circa.h>

namespace circa {
namespace dynamic_type_tests {

void test_copy()
{
    #if 0
    TEST_DISABLED
    Branch branch;
    Term* a = branch.compile("a = 5");
    Term* copy = branch.compile("copy(a)");

    evaluate_branch(branch);
    test_assert(is_int(copy));

    change_declared_type(a, STRING_TYPE);
    set_string(a, "hi");
    evaluate_branch(branch);
    test_assert(is_string(copy));
    #endif
}

void test_subroutine()
{
    Branch branch;
    branch.compile("def f(any v) -> any { return(v) }");
    test_assert(branch);

    TaggedValue* a = branch.eval("f('test')");

    test_assert(is_string(a));
    test_assert(as_string(a) == "test");

    branch.clear();
    branch.compile("def f(bool b, any v) -> any { if b { return(v) } else { return(5) } }");

    TaggedValue* b = branch.eval("f(true, 'test')");
    TaggedValue* c = branch.eval("f(false, 'test')");

    test_assert(is_string(b));
    test_assert(as_string(b) == "test");
    test_assert(is_int(c));
    test_assert(as_int(c) == 5);
}

void test_field_access()
{
    Branch branch;
    EvalContext context;

    Term* T = branch.compile("type T { int a, string b }");
    branch.compile("def f() -> any { return(T([4, 's'])) }");
    Branch& f = nested_contents(branch["f"]);
    Term* r = branch.compile("r = f()");

    test_assert(branch);
    evaluate_branch(branch);

    Term* four = f[1];
    test_assert(as_int(four) == 4);

    evaluate_branch(&context, branch);
    test_assert(context);

    branch.eval("r.a");
    Term* eq1 = branch.eval("r.a == 4");
    Term* eq2 = branch.eval("r.b == 's'");
    test_assert(context);

    test_assert(as_bool(eq1));
    test_assert(as_bool(eq2));

    test_equals(r->type->name, "any");
    test_assert(r->value_type == get_pointer(T));

    branch.compile("r.b = 's2'");
    test_assert(branch);
}

void test_subroutine_input_and_output()
{
    Branch branch;

    branch.compile("def f(Point p) { p.x }");
    branch.compile("f([1 2])");

    evaluate_branch(branch);
    test_assert(branch);

    branch.compile("def f() -> any { return([1 1] -> Point) }");
    branch.compile("a = f()");
    branch.compile("a.x");
    branch.compile("f().y");

    evaluate_branch(branch);
    test_assert(branch);
}

void test_dynamic_overload()
{
    EvalContext context;
    Branch branch;
    Term* a = create_value(branch, &ANY_T, "a");
    Term* b = create_value(branch, &ANY_T, "b");
    Term* result = branch.compile("add(a, b)");

    set_int(a, 5);
    set_int(b, 3);

    Term* add_i = get_global("add_i");
    test_assert(add_i == overloaded_function::get_overload(ADD_FUNC, 0));

    test_assert(cast_possible(a, unbox_type(INT_TYPE)));

    TermList inputs(a,b);
    test_assert(inputs_fit_function_dynamic(add_i, inputs));
    test_assert(!inputs_statically_fit_function(add_i, inputs));

    evaluate_branch(&context, branch);
    test_assert(context);
    test_assert(result->asInt() == 8);

    set_float(b, 3.0);
    test_assert(!inputs_fit_function_dynamic(add_i, inputs));

    evaluate_branch(&context, branch);
    test_assert(context);
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
