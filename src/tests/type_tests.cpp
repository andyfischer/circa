// Copyright 2008 Paul Hodge

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
    test_assert(as_type(MyType).fields.size() == 2);
    test_assert(as_type(MyType).fields[0].name == "myint");
    test_assert(as_type(MyType).fields[0].type == INT_TYPE);
    test_assert(as_type(MyType).findFieldIndex("myint") == 0);
    test_assert(as_type(MyType).fields[1].name == "astr");
    test_assert(as_type(MyType).fields[1].type == STRING_TYPE);
    test_assert(as_type(MyType).findFieldIndex("astr") == 1);

    // instanciation
    Term* inst = branch.eval("inst = MyType()");
    test_assert(inst != NULL);
    test_assert(inst->type = MyType);
    test_assert(inst->value != NULL);
    test_assert(as_branch(inst)[0]->asInt() == 0);
    test_assert(as_branch(inst)[1]->asString() == "");

    // field access on a brand new type
    Term* astr = branch.eval("inst.astr");
    test_assert(is_string(astr));
    test_equals(as_string(astr), "");

    // field assignment
    Term *inst2 = branch.eval("inst.astr = 'hello'");
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

    test_assert(as_type(myType).numFields() == 2);
    test_assert(as_type(myType).fields[0].name == "a");
    test_assert(as_type(myType).fields[0].type == STRING_TYPE);
    test_assert(as_type(myType).fields[1].name == "b");
    test_assert(as_type(myType).fields[1].type == INT_TYPE);

    test_assert(as_type(myType).alloc != NULL);
    test_assert(as_type(myType).assign != NULL);

    Term* instance = branch.eval("mt = MyType()");

    test_assert(is_value_alloced(instance));
}

void test_value_matches_type()
{
    Branch branch;

    Term* a = int_value(&branch, 5);
    test_assert(value_matches_type(a, INT_TYPE));
    test_assert(!value_matches_type(a, FLOAT_TYPE));
    test_assert(!value_matches_type(a, STRING_TYPE));
    test_assert(value_matches_type(a, ANY_TYPE));

    Term* t1 = branch.eval("type t1 { int a, float b }");
    Term* t2 = branch.eval("type t2 { int a }");
    Term* t3 = branch.eval("type t3 { int a, float b, string c }");
    Term* t4 = branch.eval("type t4 { float a, int b }");

    Term* v1 = branch.eval("[1, 2.0]");
    test_assert(value_matches_type(v1, t1));
    test_assert(!value_matches_type(v1, t2));
    test_assert(!value_matches_type(v1, t3));
    test_assert(!value_matches_type(v1, t4));

    Term* v2 = branch.eval("['hello' 2.0]");
    test_assert(!value_matches_type(v2, t1));
    test_assert(!value_matches_type(v2, t2));
    test_assert(!value_matches_type(v2, t3));
    test_assert(!value_matches_type(v2, t4));

    Term* v3 = branch.eval("[1]");
    test_assert(!value_matches_type(v3, t1));
    test_assert(value_matches_type(v3, t2));
    test_assert(!value_matches_type(v3, t3));
    test_assert(!value_matches_type(v3, t4));
    
    Term* v4 = branch.eval("[]");
    test_assert(!value_matches_type(v4, t1));
    test_assert(!value_matches_type(v4, t2));
    test_assert(!value_matches_type(v4, t3));
    test_assert(!value_matches_type(v4, t4));
}

void register_tests()
{
    REGISTER_TEST_CASE(type_tests::compound_types);
    REGISTER_TEST_CASE(type_tests::type_declaration);
    REGISTER_TEST_CASE(type_tests::test_value_matches_type);
}

} // namespace type_tests
} // namespace circa
