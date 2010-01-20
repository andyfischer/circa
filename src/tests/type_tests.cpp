// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include "circa.h"

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
    test_assert(type_t::find_field_index(MyType,"myint") == 0);
    test_assert(prototype[1]->name == "astr");
    test_assert(prototype[1]->type == STRING_TYPE);
    test_assert(type_t::find_field_index(MyType,"astr") == 1);

    test_assert(type_t::find_field_index(MyType,"the_bodies") == -1);

    // instanciation
    Term* inst = branch.eval("inst = MyType()");
    test_assert(inst != NULL);
    test_assert(inst->type = MyType);
    test_assert(inst->value.data.ptr != NULL);
    test_assert(as_branch(inst)[0]->asInt() == 0);
    test_assert(as_branch(inst)[1]->asString() == "");

    // field access on a brand new type
    Term* astr = branch.eval("inst.astr");
    test_assert(is_string(astr));
    test_equals(as_string(astr), "");

    // field assignment
    Term *inst2 = branch.eval("inst.astr = 'hello'");
    test_assert(inst2);
    test_assert(as_branch(inst2)[1]->asString() == "hello");
    test_assert(inst2->type == MyType); // type specialization

    // field access of recently assigned value
    Term* astr2 = branch.eval("inst.astr");
    test_assert(is_string(astr2));
    test_equals(as_string(astr2), "hello");
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

    test_assert(type_t::get_alloc_func(myType) != NULL);
    test_assert(type_t::get_assign_func(myType) != NULL);

    Term* instance = branch.eval("mt = MyType()");

    test_assert(is_value_alloced(instance));
}

void test_value_fits_type()
{
    Branch branch;

    Term* a = create_int(branch, 5);
    test_assert(value_fits_type(a, INT_TYPE));
    test_assert(value_fits_type(a, FLOAT_TYPE));
    test_assert(!value_fits_type(a, STRING_TYPE));
    test_assert(value_fits_type(a, ANY_TYPE));

    Term* t1 = branch.eval("type t1 { int a, number b }");
    Term* t2 = branch.eval("type t2 { int a }");
    Term* t3 = branch.eval("type t3 { int a, number b, string c }");
    Term* t4 = branch.eval("type t4 { number a, int b }");

    Term* v1 = branch.eval("[1, 2.0]");
    test_assert(value_fits_type(v1, t1));
    test_assert(!value_fits_type(v1, t2));
    test_assert(!value_fits_type(v1, t3));
    test_assert(!value_fits_type(v1, t4));

    Term* v2 = branch.eval("['hello' 2.0]");
    test_assert(!value_fits_type(v2, t1));
    test_assert(!value_fits_type(v2, t2));
    test_assert(!value_fits_type(v2, t3));
    test_assert(!value_fits_type(v2, t4));

    Term* v3 = branch.eval("[1]");
    test_assert(!value_fits_type(v3, t1));
    test_assert(value_fits_type(v3, t2));
    test_assert(!value_fits_type(v3, t3));
    test_assert(!value_fits_type(v3, t4));
    
    Term* v4 = branch.eval("[]");
    test_assert(!value_fits_type(v4, t1));
    test_assert(!value_fits_type(v4, t2));
    test_assert(!value_fits_type(v4, t3));
    test_assert(!value_fits_type(v4, t4));

    Term* v5 = branch.eval("[1 2]");
    test_assert(value_fits_type(v5, t1)); // coercion into a compound value
    test_assert(!value_fits_type(v5, t2));
    test_assert(!value_fits_type(v5, t3));
    test_assert(value_fits_type(v5, t4)); // coercion again
}

void test_is_native_type()
{
    test_assert(is_native_type(INT_TYPE));
    test_assert(is_native_type(STRING_TYPE));
    test_assert(is_native_type(BOOL_TYPE));
    test_assert(is_native_type(FLOAT_TYPE));
    test_assert(is_native_type(TYPE_TYPE));
}

void test_to_string()
{
    // Test on some native types
    test_equals(to_string(INT_TYPE), "<NativeType int>");
    test_equals(to_string(FLOAT_TYPE), "<NativeType number>");
    test_equals(to_string(BOOL_TYPE), "<NativeType bool>");
    test_equals(to_string(STRING_TYPE), "<NativeType string>");
    test_equals(to_string(TYPE_TYPE), "<NativeType Type>");

    // to_string for compound types is handled in source_repro_tests.cpp
}

void test_default_values()
{
    Branch branch;

    Term* i = create_int(branch, 5, "i");
    assign_value_to_default(i);
    test_assert(i->asInt() == 0);

    Term* f = create_float(branch, 5);
    assign_value_to_default(f);
    test_assert(f->asFloat() == 0);

    Term* s = create_string(branch, "hello");
    assign_value_to_default(s);
    test_assert(s->asString() == "");

    Term* t = create_type(branch, "T");
    type_t::get_alloc_func(t) = zero_alloc;
    type_t::get_assign_func(t) = shallow_assign;
    type_t::get_equals_func(t) = shallow_equals;
    type_t::get_is_pointer(t) = false;

#if 0
    test_assert(type_t::get_default_value(t) == NULL);

    Term* t_value = create_value(branch, t);

    set_int(t_value->value, 5);
    type_t::enable_default_value(t);
    assign_value(t_value, type_t::get_default_value(t));

    Term* t_value_2 = create_value(branch, t);

    assign_value_to_default(t_value_2);

    test_assert(equals(t_value, t_value_2));
#endif
}

void test_assign_compound_value_to_default()
{
    Branch branch;

    Term* t = branch.eval("[1 2 3]");
    assign_value_to_default(t);

    test_assert(as_branch(t).length() == 0);
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

void test_is_value_allocced()
{
    // This test was written while chasing a bug (although it turned out
    // to not capture the bug)
    Branch branch;
    Term* a = branch.eval("a = 0");
    test_assert(is_value_alloced(a));

    Term* ns = branch.eval("namespace ns; a = 0; end");
    a = ns->asBranch()[0];
    test_assert(is_value_alloced(a));

    test_assert(!type_t::get_is_pointer(INT_TYPE));
}

void test_find_common_type()
{
    test_assert(find_common_type(RefList(INT_TYPE,INT_TYPE)) == INT_TYPE);
    test_assert(find_common_type(RefList(FLOAT_TYPE,FLOAT_TYPE)) == FLOAT_TYPE);
    test_assert(find_common_type(RefList(INT_TYPE,FLOAT_TYPE)) == FLOAT_TYPE);
    test_assert(find_common_type(RefList(BOOL_TYPE,STRING_TYPE)) == ANY_TYPE);
    test_assert(find_common_type(RefList(KERNEL->get("Point"),KERNEL->get("Rect"))) == BRANCH_TYPE);
}

void _evaluate_type_error(Term* term)
{
    set_float(term, to_float(term->input(0)));
}

void test_type_error_in_a_native_call()
{
    Branch branch;

    import_function(branch, _evaluate_type_error, "f(string) -> float");

    Term* t = branch.eval("f('hello')");
    test_assert(t->hasError());
}

class test_nativeType {};

void test_imported_pointer_type()
{
    Branch branch;
    Term* T = branch.eval("type T {}");

    import_type<test_nativeType*>(branch["T"]);

    Term* v = branch.eval("v = T()");

    test_assert(is_value_of_type(v, &as_type(T)));
}

namespace simple_pointer_test {

Type* gType;
void _evaluate(Term* caller)
{
    test_assert(is_value_of_type(caller->input(0), gType));
    test_assert(is_value_of_type(caller, gType));

    test_assert(get_pointer(caller->input(0)->value, gType) == NULL);
    test_assert(get_pointer(caller->value, gType) == NULL);

    set_pointer(caller->input(0)->value, gType, NULL);
    set_pointer(caller->value, gType, NULL);
}

void test()
{
    gType = new Type();
    gType->name = "T";
    initialize_simple_pointer_type(gType);

    Branch branch;
    import_type(branch, gType);
    import_function(branch, _evaluate, "f(T) -> T");

    branch.eval("a = T()");
    branch.eval("b = f(a)");

    import_function(branch, _evaluate, "g(state T) -> T");
    branch.eval("c = g()");
}

}

void register_tests()
{
    REGISTER_TEST_CASE(type_tests::compound_types);
    REGISTER_TEST_CASE(type_tests::type_declaration);
    REGISTER_TEST_CASE(type_tests::test_value_fits_type);
    REGISTER_TEST_CASE(type_tests::test_is_native_type);
    REGISTER_TEST_CASE(type_tests::test_to_string);
    REGISTER_TEST_CASE(type_tests::test_default_values);
    REGISTER_TEST_CASE(type_tests::test_assign_compound_value_to_default);
    REGISTER_TEST_CASE(type_tests::type_inference_for_get_index);
    REGISTER_TEST_CASE(type_tests::type_inference_for_get_field);
    REGISTER_TEST_CASE(type_tests::test_is_value_allocced);
    REGISTER_TEST_CASE(type_tests::test_find_common_type);
    REGISTER_TEST_CASE(type_tests::test_type_error_in_a_native_call);
    REGISTER_TEST_CASE(type_tests::test_imported_pointer_type);
    REGISTER_TEST_CASE(type_tests::simple_pointer_test::test);
}

} // namespace type_tests
} // namespace circa
