// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa.h"
#include "importing_macros.h"

namespace circa {
namespace type_tests {

void type_declaration()
{
    Branch branch;
    Term* myType = branch.compile("type MyType { string a, int b } ");

    Branch& prototype = type_t::get_prototype(myType);
    test_assert(prototype.length() == 2);
    test_assert(prototype[0]->name == "a");
    test_assert(prototype[0]->type == STRING_TYPE);
    test_assert(prototype[1]->name == "b");
    test_assert(prototype[1]->type == INT_TYPE);

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
    test_assert(term_output_always_satisfies_type(a, unbox_type(INT_TYPE)));
    test_assert(term_output_always_satisfies_type(a, unbox_type(FLOAT_TYPE)));
    test_assert(!term_output_always_satisfies_type(a, unbox_type(STRING_TYPE)));
    test_assert(term_output_always_satisfies_type(a, unbox_type(ANY_TYPE)));

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
    test_assert(x->type == INT_TYPE);

    branch.clear();
    branch.eval("l = [1 2.0 3]");
    x = branch.compile("x = l[0]");
    test_assert(branch);
    test_assert(x->type == FLOAT_TYPE);

    branch.clear();
    branch.eval("l = [1 2.0 'three']");
    x = branch.compile("x = l[0]");
    test_assert(x->type == ANY_TYPE);
}

void type_inference_for_get_field()
{
    Branch branch;
    branch.eval("type T { int a, number b }");

    branch.eval("t = T()");
    Term* a = branch.compile("a = t.a");
    test_assert(a->type == INT_TYPE);
    Term* b = branch.compile("b = t.b");
    test_assert(b->type == FLOAT_TYPE);
}

void test_list_based_types()
{
    Branch branch;
    Term* l = branch.compile("[1 2]");

    test_assert(term_output_always_satisfies_type(l, unbox_type(LIST_TYPE)));
    test_assert(term_output_always_satisfies_type(l, unbox_type(get_global("Point"))));
}

void test_create_implicit_tuple_type()
{
    Branch branch;
    TermList types(INT_TYPE, FLOAT_TYPE, STRING_TYPE);
    Term* result = create_implicit_tuple_type(types);

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

    initialize_compound_type(unbox_type(branch["T"]));
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
}

} // namespace type_tests
} // namespace circa
