// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include <circa.h>

namespace circa {
namespace stateful_code_tests {

void test_is_get_state()
{
    Branch branch;
    Term* i = branch.compile("state int i");
    test_assert(is_get_state(i));
    test_assert(i->type == INT_TYPE);

    Term* j = branch.compile("state i = 0");
    test_assert(is_get_state(j));
}

void test_is_function_stateful()
{
    Branch branch;
    Term* f = branch.compile("def f() state s end");

    test_assert(is_function_stateful(f));

    Term* g = branch.compile("def g() 1 2 3 end");

    test_assert(!is_function_stateful(g));

    test_assert(has_implicit_state(branch.compile("f()")));
    test_assert(!has_implicit_state(branch.compile("g()")));
}

CA_FUNCTION(_empty_evaluate) {}

void test_get_type_from_branches_stateful_terms()
{
    Branch branch;
    branch.eval("a = 0");
    branch.eval("state number b");
    branch.eval("c = 'hello'");
    branch.eval("state bool d");

    Branch type;
    
    get_type_from_branches_stateful_terms(branch, type);

    test_assert(type.length() == 2);
    test_assert(is_value(type[0]));
    test_assert(type[0]->type == FLOAT_TYPE);
    test_assert(is_value(type[0]));
    test_assert(type[1]->type == BOOL_TYPE);
}

void initialize_from_expression()
{
    Branch branch;
    branch.compile("a = 1 + 2");
    branch.compile("b = a * 2");
    Term *c = branch.compile("state c = b");

    evaluate_branch(branch);

    test_equals(to_float(c), 6);
}

void one_time_assignment()
{
    // TODO
}

int NEXT_UNIQUE_OUTPUT = 0;

CA_FUNCTION(_unique_output)
{
    make_int(OUTPUT, NEXT_UNIQUE_OUTPUT++);
}

std::vector<int> SPY_RESULTS;

CA_FUNCTION(_spy)
{
    SPY_RESULTS.push_back(as_int(INPUT(0)));
}

void one_time_assignment_inside_for_loop()
{
    Branch branch;

    import_function(branch, _unique_output, "unique_output() -> int");
    import_function(branch, _spy, "spy(int)");
    branch.compile("for i in [1 1 1]\nstate s = unique_output()\nspy(s)\nend");

    NEXT_UNIQUE_OUTPUT = 0;
    SPY_RESULTS.clear();

    evaluate_branch(branch);

    test_assert(SPY_RESULTS.size() == 3);
    test_assert(SPY_RESULTS[0] == 0);
    test_assert(SPY_RESULTS[1] == 1);
    test_assert(SPY_RESULTS[2] == 2);

    SPY_RESULTS.clear();
    evaluate_branch(branch);

    test_assert(SPY_RESULTS.size() == 3);
    test_assert(SPY_RESULTS[0] == 0);
    test_assert(SPY_RESULTS[1] == 1);
    test_assert(SPY_RESULTS[2] == 2);
}

void state_and_bytecode()
{
    Branch branch;
    branch.compile("state s");
    branch.compile("s = 1");
    dump_bytecode(branch);
}

void register_tests()
{
    REGISTER_TEST_CASE(stateful_code_tests::test_is_get_state);
    REGISTER_TEST_CASE(stateful_code_tests::test_is_function_stateful);
    REGISTER_TEST_CASE(stateful_code_tests::test_get_type_from_branches_stateful_terms);
    REGISTER_TEST_CASE(stateful_code_tests::initialize_from_expression);
    REGISTER_TEST_CASE(stateful_code_tests::one_time_assignment);
    REGISTER_TEST_CASE(stateful_code_tests::one_time_assignment_inside_for_loop);
    REGISTER_TEST_CASE(stateful_code_tests::state_and_bytecode);
}

} // namespace stateful_code_tests

} // namespace circa
