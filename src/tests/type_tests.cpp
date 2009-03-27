// Copyright 2008 Andrew Fischer

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
    test_assert(as_type(myType).copy != NULL);

    Term* instance = branch.eval("mt = MyType()");

    test_assert(is_value_alloced(instance));
}

void type_iterator()
{
    Branch branch;
    Term *t = branch.eval("t = Type()");
    Type &type = as_type(t);

    ReferenceIterator *it;

    it = start_reference_iterator(t);
    test_assert(it->finished());
    delete it;

    type.addField(INT_TYPE, "int1");
    type.addField(STRING_TYPE, "int1");

    it = start_reference_iterator(t);
    test_assert(it->current() == INT_TYPE);
    it->advance();
    test_assert(it->current() == STRING_TYPE);
    it->advance();
    test_assert(it->finished());
    delete it;
}

void register_tests()
{
    REGISTER_TEST_CASE(type_tests::compound_types);
    REGISTER_TEST_CASE(type_tests::type_declaration);
    REGISTER_TEST_CASE(type_tests::type_iterator);
}

} // namespace type_tests
} // namespace circa
