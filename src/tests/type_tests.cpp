// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa.h"
#include "importing_macros.h"
#include "types/handle.h"

namespace circa {
namespace type_tests {

void type_declaration()
{
    Branch branch;
    Term* myType = branch.compile("type MyType { string a, int b } ");

    Branch& contents = nested_contents(myType);
    test_assert(contents.length() == 2);
    test_assert(contents[0]->name == "a");
    test_equals(contents[0]->type->name, "string");
    test_assert(contents[1]->name == "b");
    test_equals(contents[1]->type->name, "int");

    Type* type = unbox_type(myType);
    test_assert(type->initialize != NULL);
    test_assert(type->copy != NULL);

    branch.compile("mt = MyType()");

    test_assert(branch);
}

void test_term_output_always_satisfies_type()
{
    Branch branch;

    Term* a = create_int(branch, 5);
    test_assert(term_output_always_satisfies_type(a, &INT_T));
    test_assert(term_output_always_satisfies_type(a, &FLOAT_T));
    test_assert(!term_output_always_satisfies_type(a, &STRING_T));
    test_assert(term_output_always_satisfies_type(a, &ANY_T));

    Type* t1 = unbox_type(branch.compile("type t1 { int a, number b }"));
    Type* t2 = unbox_type(branch.compile("type t2 { int a }"));
    Type* t3 = unbox_type(branch.compile("type t3 { int a, number b, string c }"));
    Type* t4 = unbox_type(branch.compile("type t4 { number a, int b }"));

    Term* v1 = branch.compile("[1, 2.0]");
    test_assert(term_output_always_satisfies_type(v1, t1));
    test_assert(!term_output_always_satisfies_type(v1, t2));
    test_assert(!term_output_always_satisfies_type(v1, t3));
    test_assert(!term_output_always_satisfies_type(v1, t4));

    Term* v2 = branch.compile("['hello' 2.0]");
    test_assert(!term_output_always_satisfies_type(v2, t1));
    test_assert(!term_output_always_satisfies_type(v2, t2));
    test_assert(!term_output_always_satisfies_type(v2, t3));
    test_assert(!term_output_always_satisfies_type(v2, t4));

    Term* v3 = branch.compile("[1]");
    test_assert(!term_output_always_satisfies_type(v3, t1));
    test_assert(term_output_always_satisfies_type(v3, t2));
    test_assert(!term_output_always_satisfies_type(v3, t3));
    test_assert(!term_output_always_satisfies_type(v3, t4));
    
    Term* v4 = branch.compile("[]");
    test_assert(!term_output_always_satisfies_type(v4, t1));
    test_assert(!term_output_always_satisfies_type(v4, t2));
    test_assert(!term_output_always_satisfies_type(v4, t3));
    test_assert(!term_output_always_satisfies_type(v4, t4));

    Term* v5 = branch.compile("[1 2]");
    test_assert(term_output_always_satisfies_type(v5, t1));  // coercion into a compound value
    test_assert(!term_output_always_satisfies_type(v5, t2));
    test_assert(!term_output_always_satisfies_type(v5, t3));
    test_assert(term_output_always_satisfies_type(v5, t4));  // coercion again
}

void test_default_values()
{
    Branch branch;

    Term* i = create_int(branch, 5, "i");
    reset(i);
    test_assert(i->asInt() == 0);

    Term* f = create_float(branch, 5);
    reset(f);
    test_assert(f->asFloat() == 0);

    Term* s = create_string(branch, "hello");
    reset(s);
    test_assert(s->asString() == "");
}

void type_inference_for_get_index()
{
    Branch branch;
    branch.eval("l = [1 2 3]");
    Term* x = branch.compile("x = l[0]");
    test_assert(branch);
    test_equals(x->type->name, "int");

    branch.clear();
    branch.eval("l = [1 2.0 3]");
    x = branch.compile("x = l[0]");
    test_assert(branch);
    test_equals(x->type->name, "number");

    branch.clear();
    branch.eval("l = [1 2.0 'three']");
    x = branch.compile("x = l[0]");
    test_equals(x->type->name, "any");
}

void type_inference_for_get_field()
{
    Branch branch;
    branch.eval("type T { int a, number b }");

    branch.eval("t = T()");
    Term* a = branch.compile("a = t.a");
    test_equals(a->type->name, "int");
    Term* b = branch.compile("b = t.b");
    test_equals(b->type->name, "number");
}

void test_list_based_types()
{
    Branch branch;
    Term* l = branch.compile("[1 2]");

    test_assert(term_output_always_satisfies_type(l, &LIST_T));
    test_assert(term_output_always_satisfies_type(l, unbox_type(get_global("Point"))));
}

void test_create_implicit_tuple_type()
{
    Branch branch;
    List typeList;
    set_type_list(&typeList, &INT_T, &FLOAT_T, &STRING_T);
    Term* result = create_tuple_type(&typeList);

    TaggedValue* a = branch.eval("[1, 3.0, 'hi']");
    TaggedValue* b = branch.eval("['hi', 3.0, 1]");

    test_assert(cast_possible(a, unbox_type(result)));
    test_assert(!cast_possible(b, unbox_type(result)));
}

void create_empty_type_then_populate_it()
{
    Branch branch;
    branch.compile("type T;");
    branch.compile("a = T()");
    branch.compile("to_string(a)");
    test_assert(branch);
    evaluate_branch(branch);

    list_t::setup_type(unbox_type(branch["T"]));
    evaluate_branch(branch);
    test_assert(branch);
}

void test_copy_builtin_type()
{
    TaggedValue v;
    copy(INT_TYPE, &v);
    test_assert(v.value_type == &TYPE_T);

    Branch branch;
    branch.eval("copy(int)");
    test_assert(branch);
}

void test_compound_type_cast()
{
    List a;
    a.resize(2);
    set_int(a[0], 0);
    set_int(a[1], 1);

    test_equals(&a, "[0, 1]");

    TaggedValue b;
    cast(&a, unbox_type(get_global("Point")), &b);
    test_equals(&b, "[0.0, 1.0]");
}

void test_declaring_term()
{
    Branch branch;
    Term* myType = branch.compile("type MyType {}");
    TaggedValue myValue;
    change_type(&myValue, as_type(myType));
    test_equals(myValue.value_type->name, "MyType");
    test_assert(myValue.value_type->declaringTerm == myType);

    clear_branch(&branch);
    test_equals(myValue.value_type->name, "MyType");
    test_assert(myValue.value_type->declaringTerm == NULL);
}

Type *g_testHandleType;

CA_FUNCTION(test_handle_function)
{
    handle_t::set(OUTPUT, g_testHandleType, (void*) NULL);
}

void use_installed_handle_type()
{
    Branch branch;

    Term* myType = branch.compile("type MyType {}");
    g_testHandleType = as_type(myType);
    Term* f = branch.compile("def f() -> MyType { return [] }");

    handle_t::setup_type(g_testHandleType);
    install_function(f, test_handle_function);

    Term* fCall = branch.compile("f()");

    evaluate_branch(branch);
    test_assert(cast_possible(fCall, as_type(myType)));
}

void register_tests()
{
    REGISTER_TEST_CASE(type_tests::type_declaration);
    REGISTER_TEST_CASE(type_tests::test_term_output_always_satisfies_type);
    REGISTER_TEST_CASE(type_tests::test_default_values);
    REGISTER_TEST_CASE(type_tests::type_inference_for_get_index);
    REGISTER_TEST_CASE(type_tests::type_inference_for_get_field);
    REGISTER_TEST_CASE(type_tests::test_list_based_types);
    REGISTER_TEST_CASE(type_tests::test_create_implicit_tuple_type);
    REGISTER_TEST_CASE(type_tests::create_empty_type_then_populate_it);
    REGISTER_TEST_CASE(type_tests::test_copy_builtin_type);
    REGISTER_TEST_CASE(type_tests::test_compound_type_cast);
    REGISTER_TEST_CASE(type_tests::test_declaring_term);
    REGISTER_TEST_CASE(type_tests::use_installed_handle_type);
}

} // namespace type_tests
} // namespace circa
