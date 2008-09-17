// Copyright 2008 Andrew Fischer

#include "tests/common.h"
#include "common_headers.h"
#include "branch.h"
#include "builtins.h"
#include "compound_type.h"
#include "operations.h"
#include "parser.h"
#include "type.h"

namespace circa {
namespace type_tests {

void derived_type()
{
    Branch branch;

    Term* type1 = create_constant(&branch, TYPE_TYPE);
    as_type(type1)->dataSize = 6;

    test_assert(as_type(type1)->getInstanceOffset() == 0);

    Term* derivedType = create_constant(&branch, TYPE_TYPE);
    as_type(derivedType)->dataSize = 12;
    as_type(derivedType)->parentType = type1;
    test_assert(as_type(derivedType)->getInstanceOffset() == 6);

    Term* derivedType2 = create_constant(&branch, TYPE_TYPE);
    as_type(derivedType2)->dataSize = 3;
    as_type(derivedType2)->parentType = derivedType;
    test_assert(as_type(derivedType2)->getInstanceOffset() == 18);
}

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
}

/*
void test_fields()
{
    Branch branch;
    Term* mytype = create_empty_type(&branch);
    as_type(mytype)->addField("field-name", INT_TYPE);

    Term* myinst = create_constant(&branch, mytype);

    test_assert(myinst->field("field-name") != NULL);
    test_assert(myinst->field("field-name")->type == INT_TYPE);

    // nonexistant field
    test_assert(myinst->field("other-name") == NULL);
}

void test_set_field()
{
    Branch branch;
    Term* mytype = create_empty_type(&branch);
    as_type(mytype)->addField("field-name", INT_TYPE);

    Term* myterm = create_constant(&branch, mytype);
    branch.bindName(myterm, "myterm");

    eval_statement(branch, "set-field(@myterm, 'field-name, 15)");
    test_assert(branch["myterm"] != NULL);
    //Doesn't work yet, need parametric types
    //test_assert(branch["myterm"]->fields["field-name"] != NULL);
    //test_assert(branch["myterm"]->fields["field-name"]->asInt() == 15);
}
*/

} // namespace type_tests

void register_type_tests()
{
    REGISTER_TEST_CASE(type_tests::derived_type);
    REGISTER_TEST_CASE(type_tests::compound_type);
    //REGISTER_TEST_CASE(type_tests::test_fields);
    //REGISTER_TEST_CASE(type_tests::test_set_field);
}

} // namespace circa
