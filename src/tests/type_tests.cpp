// Copyright 2008 Andrew Fischer

#include "tests/common.h"
#include "common_headers.h"
#include "branch.h"
#include "builtins.h"
#include "operations.h"
#include "parser.h"
#include "type.h"

namespace circa {
namespace type_tests {

    /*

void compound_type()
{
    CompoundType ctype;
    ctype.addField(INT_TYPE, "int1");
    test_assert(ctype.getType(0) == INT_TYPE);
    test_assert(ctype.getName(0) == "int1");

    Branch branch;
    Term* t;
    t = eval_statement(branch, "t = CompoundType()");
    test_assert(as_compound_type(t).numFields() == 0);
    t = eval_statement(branch, "compound-type-add-field(@t, int, 'myfield)");
    test_assert(as_compound_type(t).getType(0) == INT_TYPE);
    test_assert(as_compound_type(t).getName(0) == "myfield");
    t = eval_statement(branch, "compound-type-add-field(@t, bool, \"a bool field\")");
    test_assert(as_compound_type(t).getType(1) == BOOL_TYPE);
    test_assert(as_compound_type(t).getName(1) == "a bool field");
}

void test_instantiate_compound_value()
{
    Branch branch;
    Term* t;
    t = eval_statement(branch, "t = CompoundType()");
    t = eval_statement(branch, "compound-type-add-field(@t, int, 'a)");
    t = eval_statement(branch, "compound-type-add-field(@t, string, 'b)");

    CompoundValue value;

    instantiate_compound_value(as_compound_type(t), value);

    test_assert(value.getField(0)->type == INT_TYPE);
    test_assert(value.getField(1)->type == STRING_TYPE);

    t = eval_statement(branch, "t = CompoundType()");
    t = eval_statement(branch, "compound-type-add-field(@t, float, 'a)");
    t = eval_statement(branch, "compound-type-add-field(@t, float, 'b)");

    instantiate_compound_value(as_compound_type(t), value);

    test_assert(value.getField(0)->type == FLOAT_TYPE);
    test_assert(value.getField(1)->type == FLOAT_TYPE);
}
*/

void bootstrapped_objects()
{
    test_assert(get_parent_type(COMPOUND_TYPE) == TYPE_TYPE);
    test_assert(get_parent(COMPOUND_TYPE)->type == TYPE_TYPE);

    test_assert(get_as(COMPOUND_TYPE, COMPOUND_TYPE) == COMPOUND_TYPE);

    test_assert(get_as(COMPOUND_TYPE, TYPE_TYPE) != NULL);
    test_assert(get_as(COMPOUND_TYPE, TYPE_TYPE)->type == TYPE_TYPE);

    test_assert(as_type(COMPOUND_TYPE) != NULL);

    test_assert(get_field(COMPOUND_TYPE, "parent") == get_parent(COMPOUND_TYPE));
    test_assert(get_field(COMPOUND_TYPE, "fields") != NULL);
}

void compound_types()
{
    Branch branch;

    Term* type1 = eval_statement(branch, "type1 = create-compound-type('type1)");
    test_assert(type1 != NULL);
    test_assert(get_field(type1, "parent")->type == TYPE_TYPE);
    test_assert(get_field(type1, "fields")->type == LIST_TYPE);
    test_assert(is_type(type1));

    Term* type2 = eval_statement(branch, "type2 = create-compound-type('type2)");
    test_assert(type2 != NULL);
    test_assert(get_field(type2, "parent")->type == TYPE_TYPE);
    test_assert(get_field(type2, "fields")->type == LIST_TYPE);
    test_assert(get_field(type1, "parent") != get_field(type2, "parent"));
    test_assert(get_field(type1, "fields") != get_field(type2, "fields"));

    test_assert(is_type(type1));
    eval_statement(branch, "compound-type-append-field(@type1, int, 'myint)");
    return;
    test_assert(is_type(type1));
    eval_statement(branch, "compound-type-append-field(@type1, string, 'mystring)");
    test_assert(is_type(type1));

    Term* inst1 = eval_statement(branch, "inst1 = type1()");
    test_assert(get_field(inst1, "myint") != NULL);
    test_assert(get_field(inst1, "myint")->type == INT_TYPE);
    test_assert(get_field(inst1, "mystring") != NULL);
    test_assert(get_field(inst1, "mystring")->type == INT_TYPE);
}

} // namespace type_tests

void register_type_tests()
{
    REGISTER_TEST_CASE(type_tests::bootstrapped_objects);
    REGISTER_TEST_CASE(type_tests::compound_types);
}

} // namespace circa
