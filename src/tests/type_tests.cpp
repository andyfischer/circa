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

void bootstrapped_objects()
{
    //FIXME test_assert(get_parent_type(COMPOUND_TYPE) == TYPE_TYPE);
    //FIXME test_assert(get_parent(COMPOUND_TYPE)->type == TYPE_TYPE);

    test_assert(get_as(COMPOUND_TYPE, COMPOUND_TYPE) == COMPOUND_TYPE);

    test_assert(get_as(COMPOUND_TYPE, TYPE_TYPE) != NULL);
    test_assert(get_as(COMPOUND_TYPE, TYPE_TYPE)->type == TYPE_TYPE);

    test_assert(as_type(COMPOUND_TYPE) != NULL);

    //FIXME test_assert(get_field(COMPOUND_TYPE, "parent") == get_parent(COMPOUND_TYPE));
    test_assert(get_field(COMPOUND_TYPE, "fields") != NULL);

    //std::cout << "COMPOUND_TYPE = " << COMPOUND_TYPE->toString() << std::endl;

    Branch branch;
    Term* typeA = eval_statement(branch, "typeA = CompoundType()");

    eval_statement(branch, "compound-type-append-field(@typeA, int, 'I)");
    typeA = eval_statement(branch, "compound-type-append-field(@typeA, string, 'S)");

    //std::cout << "typeA = " << typeA->toString() << std::endl;

    Term* typeAinstance = eval_statement(branch, "typeAinstance = typeA()");

    //std::cout << "typeAinstance = " << typeAinstance->toString() << std::endl;
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
    //std::cout << type1->toString() << std::endl;
    type1 = eval_statement(branch, "compound-type-append-field(@type1, int, 'myint)");
    //std::cout << type1->toString() << std::endl;
    test_assert(is_type(type1));
    type1 = eval_statement(branch, "compound-type-append-field(@type1, string, 'mystring)");
    test_assert(is_type(type1));

    Term* inst1 = eval_statement(branch, "inst1 = type1()");
    test_assert(get_field(inst1, "myint") != NULL);
    test_assert(get_field(inst1, "myint")->type == INT_TYPE);
    test_assert(get_field(inst1, "mystring") != NULL);
    test_assert(get_field(inst1, "mystring")->type == STRING_TYPE);
}

} // namespace type_tests

void register_type_tests()
{
    REGISTER_TEST_CASE(type_tests::bootstrapped_objects);
    REGISTER_TEST_CASE(type_tests::compound_types);
}

} // namespace circa
