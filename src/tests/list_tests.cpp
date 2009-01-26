// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "testing.h"
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

    Term* list1 = branch.eval("list1 = list(1, 'pie', 2.0)");

    test_assert(list1->type == LIST_TYPE);
    test_assert(as_type(list1->type).toString != NULL);
    test_assert(list1->toString() == "[1, pie, 2]");

    test_assert(list1->asList()[0]->asInt() == 1);
    test_assert(list1->asList()[0]->asInt() != 2);
    test_assert(list1->asList()[1]->asString() == "pie");
    test_assert(list1->asList()[2]->asFloat() == 2.0);

    // make sure items are copied by value
    branch.eval("a = 5")->stealingOk = false;
    Term* list2 = branch.eval("list2 = list(a)");
    test_assert(list2->asList()[0]->asInt() == 5);
    branch["a"]->asInt() = 6;
    test_assert(list2->asList()[0]->asInt() == 5);
}

void operations()
{
    Branch branch;

    Term* list1 = branch.eval("list1 = list('pie')");
    Term* list2 = branch.eval("list2 = list()");

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

    Term* range_zero_to_ten = branch.eval("range(10)");

    test_assert(as_int(as_list(range_zero_to_ten).get(0)) == 0);
    test_assert(as_int(as_list(range_zero_to_ten).get(9)) == 9);
}

void list_apply()
{
    Branch branch;

    Term* result = branch.eval("list-apply(to-string, range(5))");
    
    test_assert(as_string(as_list(result).get(0)) == "0");
    test_assert(as_string(as_list(result).get(4)) == "4");
}

void register_tests()
{
    REGISTER_TEST_CASE(list_tests::create);
    REGISTER_TEST_CASE(list_tests::operations);
    REGISTER_TEST_CASE(list_tests::range);
    REGISTER_TEST_CASE(list_tests::list_apply);
}

} // namespace list_tests

} // namespace circa
