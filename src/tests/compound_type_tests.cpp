// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include "circa.h"
#include "importing_macros.h"

namespace circa {
namespace compound_type_tests {

void compound_type_usage()
{
    Branch branch;

    Term* MyType = branch.compile("type MyType { int myint, string astr }");
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
    //TEST_DISABLED test_assert(inst2->type == MyType); // type specialization

    test_assert(is_string(astr2));
    test_equals(as_string(astr2), "hello");
}

void test_cast()
{
    Branch branch;
    Term* t = branch.compile("type T { string a }");
    TaggedValue* a = branch.eval("['hi']");
    test_assert(is_list(a));
    test_assert(a->value_type != unbox_type(t));

    TaggedValue casted;
    test_assert(cast(a, unbox_type(t), &casted));
    test_assert(is_list(&casted));
    test_assert(casted.value_type == unbox_type(t));
}

void test_bug_with_cast()
{
    // trying to repro a Plastic bug
    List value;
    set_int(value.append(), 1);
    set_int(value.append(), 72);
    set_int(value.append(), 18);

    test_equals(&value, "[1, 72, 18]");

    Branch branch;
    Term* type = branch.compile("type T {int x, int y, int z}");

    TaggedValue castResult;
    cast(&value, unbox_type(type), &castResult);

    test_equals(&castResult, "[1, 72, 18]");

    cast(&castResult, unbox_type(type), &castResult);

    test_equals(&castResult, "[1, 72, 18]");
}

void test_bug_with_change_type()
{

}
    
void register_tests()
{
    REGISTER_TEST_CASE(compound_type_tests::compound_type_usage);
    REGISTER_TEST_CASE(compound_type_tests::test_cast);
    REGISTER_TEST_CASE(compound_type_tests::test_bug_with_cast);
}

} // namespace compound_type_tests
} // namespace circa
