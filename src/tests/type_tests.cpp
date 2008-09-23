// Copyright 2008 Paul Hodge

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

}

} // namespace type_tests

void register_type_tests()
{
    REGISTER_TEST_CASE(type_tests::bootstrapped_objects);
    //REGISTER_TEST_CASE(type_tests::compound_type);
    //REGISTER_TEST_CASE(type_tests::test_instantiate_compound_value);
}

} // namespace circa
