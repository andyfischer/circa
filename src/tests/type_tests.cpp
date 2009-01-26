// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "builtin_types.h"
#include "branch.h"
#include "builtins.h"
#include "runtime.h"
#include "parser.h"
#include "testing.h"
#include "type.h"

namespace circa {
namespace type_tests {

void compound_types()
{
    Branch branch;

    Term* type1 = branch.eval("type1 = create-compound-type('type1')");
    test_assert(type1 != NULL);
    test_assert(is_type(type1));

    type1 = branch.eval("compound-type-append-field(@type1, int, 'myint')");
    test_assert(is_type(type1));
    test_assert(as_type(type1).fields.size() == 1);
    test_assert(as_type(type1).fields[0].name == "myint");
    test_assert(as_type(type1).fields[0].type == INT_TYPE);
    test_assert(as_type(type1).findField("myint") == 0);

    type1 = branch.eval("compound-type-append-field(@type1, string, 'astr')");
    test_assert(is_type(type1));
    test_assert(as_type(type1).fields.size() == 2);
    test_assert(as_type(type1).fields[1].name == "astr");
    test_assert(as_type(type1).fields[1].type == STRING_TYPE);
    test_assert(as_type(type1).findField("astr") == 1);

    Term* inst1 = branch.eval("inst1 = type1()");
    test_assert(inst1 != NULL);
    test_assert(inst1->type = type1);
    test_assert(inst1->value != NULL);

    // test get-field function
    Term* inst1_myint = branch.eval("get-field(inst1, 'myint')");
    test_assert(inst1_myint != NULL);
    std::cout << inst1_myint->getErrorMessage() << std::endl;
    test_assert(!inst1_myint->hasError());

    Term* inst1_astr = branch.eval("get-field(inst1, 'astr')");
    test_assert(inst1_astr != NULL);
    test_assert(!inst1_astr->hasError());
    
    // test get_field
    test_assert(get_field(inst1,"myint") != NULL);
    test_assert(get_field(inst1,"myint")->value != NULL);
    get_field(inst1,"myint")->asInt() = 5;
    branch.eval("get-field(inst1, 'myint')");
    test_assert(branch.eval("get-field(inst1, 'myint')")->asInt() == 5);

    Term* inst2 = branch.eval("inst2 = type1()");

    test_assert(get_field(inst1,"myint") != get_field(inst2,"myint"));
    get_field(inst2,"myint")->asInt() = 7;
    test_assert(get_field(inst1,"myint")->asInt() == 5);
}


void type_iterator()
{
    Branch branch;
    Term *t = branch.eval("t = Type()");
    Type &type = as_type(t);

    PointerIterator *it;

    it = start_pointer_iterator(t);
    test_assert(it->finished());
    delete it;

    type.addField(INT_TYPE, "int1");
    type.addField(STRING_TYPE, "int1");

    it = start_pointer_iterator(t);
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
    REGISTER_TEST_CASE(type_tests::type_iterator);
}

} // namespace type_tests
} // namespace circa
