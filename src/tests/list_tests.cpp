// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "tests/common.h"
#include "branch.h"
#include "builtins.h"
#include "list.h"
#include "parser.h"
#include "term.h"
#include "type.h"
#include "values.h"

namespace circa {
namespace list_tests {

void create()
{
    Branch branch;

    Term* list1 = eval_statement(branch, "list1 = list(1, 'pie', 2.0)");

    test_assert(list1->type == LIST_TYPE);
    test_assert(as_type(list1->type).toString != NULL);
    test_assert(list1->toString() == "[1, pie, 2]");

    test_assert(list1->asList()[0]->asInt() == 1);
    test_assert(list1->asList()[0]->asInt() != 2);
    test_assert(list1->asList()[1]->asString() == "pie");
    test_assert(list1->asList()[2]->asFloat() == 2.0);

    // make sure items are copied by value
    eval_statement(branch, "a = 5")->stealingOk = false;
    Term* list2 = eval_statement(branch, "list2 = list(a)");
    test_assert(list2->asList()[0]->asInt() == 5);
    branch["a"]->asInt() = 6;
    test_assert(list2->asList()[0]->asInt() == 5);
}

void operations()
{
    Branch branch;

    Term* list1 = eval_statement(branch, "list1 = list('pie')");
    Term* list2 = eval_statement(branch, "list2 = list()");

    duplicate_value(list1, list2);

    test_assert(list1->asList()[0]->asString() == "pie");
    test_assert(list2->asList()[0]->asString() == "pie");
    list1->asList()[0]->asString() = "cake";
    test_assert(list2->asList()[0]->asString() == "pie");

    steal_value(list1, list2);

    test_assert(list1->value == NULL);
    test_assert(list2->asList()[0]->asString() == "cake");
}

void range()
{
    Branch branch;

    Term* range_zero_to_ten = eval_statement(branch, "range(10)");

    test_assert(as_int(as_list(range_zero_to_ten).get(0)) == 0);
    test_assert(as_int(as_list(range_zero_to_ten).get(9)) == 9);
}

void list_apply()
{
    Branch branch;

    Term* result = eval_statement(branch, "list-apply(to-string, range(5))");
    
    test_assert(as_string(as_list(result).get(0)) == "0");
    test_assert(as_string(as_list(result).get(4)) == "4");
}

void to_reference_list()
{
    List list;

    list.appendSlot(REFERENCE_TYPE)->asRef() = INT_TYPE;
    list.appendSlot(REFERENCE_TYPE)->asRef() = CONSTANT_FALSE;
    list.appendSlot(REFERENCE_TYPE)->asRef() = APPLY_FEEDBACK;

    ReferenceList reflist = list.toReferenceList();

    test_assert(reflist.count() == 3);
    test_assert(reflist[0] == INT_TYPE);
    test_assert(reflist[1] == CONSTANT_FALSE);
    test_assert(reflist[2] == APPLY_FEEDBACK);
}

void get_references()
{
    Branch branch;

    eval_statement(branch, "l = list(int, any, false)");
    Term* rl = eval_statement(branch, "get-list-references(l)");

    test_assert(as_list(rl)[0]->asRef() == INT_TYPE);
    test_assert(as_list(rl)[1]->asRef() == ANY_TYPE);
    test_assert(as_list(rl)[2]->asRef() == CONSTANT_FALSE);

    ReferenceList reflist = as_list(rl).toReferenceList();

    test_assert(reflist[0] == INT_TYPE);
    test_assert(reflist[1] == ANY_TYPE);
    test_assert(reflist[2] == CONSTANT_FALSE);
}

} // namespace list_tests

void register_list_tests()
{
    REGISTER_TEST_CASE(list_tests::create);
    REGISTER_TEST_CASE(list_tests::operations);
    REGISTER_TEST_CASE(list_tests::range);
    REGISTER_TEST_CASE(list_tests::list_apply);
    REGISTER_TEST_CASE(list_tests::to_reference_list);
    REGISTER_TEST_CASE(list_tests::get_references);
}

} // namespace circa
