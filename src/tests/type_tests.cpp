// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include "circa.h"
#include "importing_macros.h"

namespace circa {
namespace type_tests {

void compound_types()
{
    Branch branch;

    Term* MyType = branch.eval("type MyType { int myint, string astr }");
    test_assert(MyType != NULL);
    test_assert(is_type(MyType));
    Branch& prototype = type_t::get_prototype(MyType);
    test_assert(prototype.length() == 2);
    test_assert(prototype[0]->name == "myint");
    test_assert(prototype[0]->type == INT_TYPE);
    test_assert(as_type(MyType).findFieldIndex("myint") == 0);
    test_assert(prototype[1]->name == "astr");
    test_assert(prototype[1]->type == STRING_TYPE);
    test_assert(as_type(MyType).findFieldIndex("astr") == 1);

    test_assert(as_type(MyType).findFieldIndex("the_bodies") == -1);

    // instanciation
    Term* inst = branch.compile("inst = MyType()");

    // field access on a brand new type
    Term* astr = branch.compile("inst.astr");

    // field assignment
    Term *inst2 = branch.compile("inst.astr = 'hello'");

    // field access of recently assigned value
    Term* astr2 = branch.compile("inst.astr");

    evaluate_branch(branch);

    test_assert(inst->type = MyType);
    test_assert(inst->value_data.ptr != NULL);
    test_assert(inst->getIndex(0)->asInt() == 0);
    test_assert(inst->getIndex(1)->asString() == "");

    test_assert(is_string(astr));
    test_equals(as_string(astr), "");

    test_equals(inst2->getIndex(1)->asString(), "hello");
    test_assert(inst2->type == MyType); // type specialization

    test_assert(is_string(astr2));
    test_equals(as_string(astr2), "hello");
}

void compound_type_cast()
{
    Branch branch;
    Term* t = branch.eval("type T { string a }");
    Term* a = branch.eval("['hi']");
    test_assert(is_list(a));
    test_assert(a->value_type != type_contents(t));

    TaggedValue casted;
    test_assert(cast(a, type_contents(t), &casted));
    test_assert(is_list(&casted));
    test_assert(casted.value_type == type_contents(t));
}

void type_declaration()
{
    Branch branch;
    Term* myType = branch.eval("type MyType { string a, int b } ");

    Branch& prototype = type_t::get_prototype(myType);
    test_assert(prototype.length() == 2);
    test_assert(prototype[0]->name == "a");
    test_assert(prototype[0]->type == STRING_TYPE);
    test_assert(prototype[1]->name == "b");
    test_assert(prototype[1]->type == INT_TYPE);

    Type* type = &as_type(myType);
    test_assert(type->initialize != NULL);
    test_assert(type->copy != NULL);

    branch.eval("mt = MyType()");

    test_assert(branch);
}

void test_term_output_always_satisfies_type()
{
    Branch branch;

    Term* a = create_int(branch, 5);
    test_assert(term_output_always_satisfies_type(a, type_contents(INT_TYPE)));
    test_assert(term_output_always_satisfies_type(a, type_contents(FLOAT_TYPE)));
    test_assert(!term_output_always_satisfies_type(a, type_contents(STRING_TYPE)));
    test_assert(term_output_always_satisfies_type(a, type_contents(ANY_TYPE)));

    Type* t1 = type_contents(branch.eval("type t1 { int a, number b }"));
    Type* t2 = type_contents(branch.eval("type t2 { int a }"));
    Type* t3 = type_contents(branch.eval("type t3 { int a, number b, string c }"));
    Type* t4 = type_contents(branch.eval("type t4 { number a, int b }"));

    Term* v1 = branch.eval("[1, 2.0]");
    test_assert(term_output_always_satisfies_type(v1, t1));
    test_assert(!term_output_always_satisfies_type(v1, t2));
    test_assert(!term_output_always_satisfies_type(v1, t3));
    test_assert(!term_output_always_satisfies_type(v1, t4));

    Term* v2 = branch.eval("['hello' 2.0]");
    test_assert(!term_output_always_satisfies_type(v2, t1));
    test_assert(!term_output_always_satisfies_type(v2, t2));
    test_assert(!term_output_always_satisfies_type(v2, t3));
    test_assert(!term_output_always_satisfies_type(v2, t4));

    Term* v3 = branch.eval("[1]");
    test_assert(!term_output_always_satisfies_type(v3, t1));
    test_assert(term_output_always_satisfies_type(v3, t2));
    test_assert(!term_output_always_satisfies_type(v3, t3));
    test_assert(!term_output_always_satisfies_type(v3, t4));
    
    Term* v4 = branch.eval("[]");
    test_assert(!term_output_always_satisfies_type(v4, t1));
    test_assert(!term_output_always_satisfies_type(v4, t2));
    test_assert(!term_output_always_satisfies_type(v4, t3));
    test_assert(!term_output_always_satisfies_type(v4, t4));

    Term* v5 = branch.eval("[1 2]");
    test_assert(term_output_always_satisfies_type(v5, t1));  // coercion into a compound value
    test_assert(!term_output_always_satisfies_type(v5, t2));
    test_assert(!term_output_always_satisfies_type(v5, t3));
    test_assert(term_output_always_satisfies_type(v5, t4));  // coercion again
}

void test_is_native_type()
{
    test_assert(is_native_type(INT_TYPE));
    test_assert(is_native_type(STRING_TYPE));
    test_assert(is_native_type(BOOL_TYPE));
    test_assert(is_native_type(FLOAT_TYPE));
    test_assert(is_native_type(TYPE_TYPE));
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

    //create_type(branch, "T");

#if 0
    // FIXME
    test_assert(type_t::get_default_value(t) == NULL);

    Term* t_value = create_value(branch, t);

    set_int(t_value->value, 5);
    type_t::enable_default_value(t);
    assign_value(t_value, type_t::get_default_value(t));

    Term* t_value_2 = create_value(branch, t);

    reset(t_value_2);

    test_assert(equals(t_value, t_value_2));
#endif
}

void test_assign_compound_value_to_default()
{
    Branch branch;

    Term* t = branch.eval("[1 2 3]");
    reset(t);

    //FIXME
    //test_assert(t->numElements() == 0);
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

    test_assert(term_output_always_satisfies_type(l, type_contents(LIST_TYPE)));
    test_assert(term_output_always_satisfies_type(l, type_contents(get_global("Point"))));
}

void test_create_implicit_tuple_type()
{
    Branch branch;
    RefList types(INT_TYPE, FLOAT_TYPE, STRING_TYPE);
    Term* result = create_implicit_tuple_type(types);

    Term* a = branch.eval("[1, 3.0, 'hi']");
    Term* b = branch.eval("['hi', 3.0, 1]");

    test_assert(value_fits_type(a, type_contents(result)));
    test_assert(!value_fits_type(b, type_contents(result)));
}

void register_tests()
{
    REGISTER_TEST_CASE(type_tests::compound_types);
    REGISTER_TEST_CASE(type_tests::compound_type_cast);
    REGISTER_TEST_CASE(type_tests::type_declaration);
    REGISTER_TEST_CASE(type_tests::test_term_output_always_satisfies_type);
    REGISTER_TEST_CASE(type_tests::test_is_native_type);
    REGISTER_TEST_CASE(type_tests::test_default_values);
    REGISTER_TEST_CASE(type_tests::test_assign_compound_value_to_default);
    REGISTER_TEST_CASE(type_tests::type_inference_for_get_index);
    REGISTER_TEST_CASE(type_tests::type_inference_for_get_field);
    REGISTER_TEST_CASE(type_tests::test_list_based_types);
    REGISTER_TEST_CASE(type_tests::test_create_implicit_tuple_type);
}

} // namespace type_tests
} // namespace circa
