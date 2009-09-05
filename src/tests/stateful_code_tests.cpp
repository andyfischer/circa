// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "common_headers.h"

#include <circa.h>

namespace circa {
namespace stateful_code_tests {

void test_simple()
{
    Branch branch;
    Term* i = branch.eval("state int i");
    test_assert(is_stateful(i));
    test_assert(i->type == INT_TYPE);

    Term* j = branch.eval("state i = 0");
    test_assert(is_stateful(j));
    test_assert(j->type == INT_TYPE);
}

void _empty_evaluate(Term*) {}

void function_with_hidden_state_term()
{
    Branch branch;
    Term* myfunc = import_function(branch, _empty_evaluate, "myfunc(state int)");
    Term* call_1 = branch.eval("myfunc()");
    Term* call_1_state = get_hidden_state_for_call(call_1);

    test_assert(branch[0] == myfunc);
    test_assert(branch[1] == call_1_state);
    test_assert(branch[2] == call_1);
    test_assert(is_stateful(call_1_state));
    test_assert(!is_stateful(call_1));

    Term* call_2 = branch.eval("myfunc()");
    Term* call_2_state = get_hidden_state_for_call(call_2);

    test_assert(is_stateful(call_2_state));
    test_assert(!is_stateful(call_2));
    test_assert(call_1_state != call_2_state);
}

void subroutine_expansion_during_migrate()
{
    Branch branch;

    branch.eval("def myfunc(); state int i; end");

    Branch& source = create_branch(branch, "source");
    Branch& dest = create_branch(branch, "dest");

    Term* sourceCall = source.eval("myfunc()");
    Term* destCall = dest.compile("myfunc()");

    Term* sourceCallState = get_hidden_state_for_call(sourceCall);
    Term* destCallState = get_hidden_state_for_call(destCall);

    test_assert(is_subroutine_state_expanded(sourceCallState));
    test_assert(!is_subroutine_state_expanded(destCallState));

    sourceCallState->asBranch()["i"]->asInt() = 111;

    migrate_stateful_values(source, dest);

    //test_assert(is_subroutine_state_expanded(get_hidden_state_for_call(destCall)));
    test_assert(as_branch(destCallState).contains("i"));
    test_assert(as_branch(destCallState)["i"]->asInt() == 111);
}

void test_load_and_save()
{
    Branch branch;
    Term* statefulTerm = branch.eval("state int i");
    as_int(statefulTerm) = 1;

    Branch state;
    Term* value_i = create_value(state, INT_TYPE, "i");
    as_int(value_i) = 5;

    test_assert(as_int(statefulTerm) == 1);

    load_state_into_branch(state, branch);

    test_assert(as_int(statefulTerm) == 5);

    as_int(statefulTerm) = 11;

    persist_state_from_branch(branch, state);

    test_assert(as_int(value_i) == 11);
}

void test_get_type_from_branches_stateful_terms()
{
    Branch branch;
    branch.eval("a = 0");
    branch.eval("state float b");
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

void stateful_value_evaluation()
{
    Branch branch;
    Term *i = branch.eval("state i = 2.0");

    branch.compile("i = i + 1.0");

    test_equals(as_float(i), 2.0);
    evaluate_branch(branch);
    test_equals(as_float(i), 3.0);
    evaluate_branch(branch);
    test_equals(as_float(i), 4.0);
}

void initialize_from_expression()
{
    Branch branch;
    branch.eval("a = 1 + 2");
    branch.eval("b = a * 2");
    Term *c = branch.eval("state c = b");

    test_equals(to_float(c), 6);
}

void one_time_assignment()
{
    Branch branch;
    Term* a = int_value(branch, 3, "a");
    Term* s = branch.compile("state s = a");

    // change value before evaluate_branch, to make sure it's not stored
    // at compile time.
    as_int(a) = 5;
    evaluate_branch(branch);
    test_assert(s);
    test_assert(as_int(s) == 5);

    // now make sure a subsequent evaluation doesn't change 's'
    as_int(a) = 7;
    evaluate_branch(branch);
    test_assert(s);
    test_assert(as_int(s) == 5);
    as_int(a) = 9;
    evaluate_branch(branch);
    test_assert(s);
    test_assert(as_int(s) == 5);
}

int NEXT_UNIQUE_OUTPUT = 0;

void _unique_output(Term* caller)
{
    as_int(caller) = NEXT_UNIQUE_OUTPUT;
    NEXT_UNIQUE_OUTPUT++;
}

std::vector<int> SPY_RESULTS;

void _spy(Term* caller)
{
    SPY_RESULTS.push_back(as_int(caller->input(0)));
}

void one_time_assignment_inside_for_loop()
{
    Branch branch;

    import_function(branch, _unique_output, "unique_output() : int");
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

void state_inside_lots_of_nested_functions()
{
    Branch branch;

    import_function(branch, _unique_output, "unique_output() : int");
    import_function(branch, _spy, "spy(int)");

    Term* f1 = branch.compile("def func1()\nstate s = unique_output()\nspy(s)\nend");
    Term* f2 = branch.compile("def func2()\nfunc1()\nend");
    Term* f3 = branch.compile("def func3()\nfunc2()\nend");
    Term* f4 = branch.compile("def func4()\nfunc3()\nend");

    test_assert(is_function_stateful(f1));
    test_assert(is_function_stateful(f2));
    test_assert(is_function_stateful(f3));
    test_assert(is_function_stateful(f4));

    NEXT_UNIQUE_OUTPUT = 11;
    SPY_RESULTS.clear();

    Term* call = branch.compile("func4()");
    test_assert(call->numInputs() == 1);
    evaluate_branch(branch);

    test_assert(SPY_RESULTS.size() == 1);
    test_assert(SPY_RESULTS[0] == 11);

    SPY_RESULTS.clear();

    evaluate_branch(branch);

    test_assert(SPY_RESULTS.size() == 1);
    test_assert(SPY_RESULTS[0] == 11);
}

void migrate_subroutine_with_no_hidden_state()
{
    Branch source;
    source.eval("def hello()\nend");
    source.eval("hello()");

    Branch dest;
    dest.eval("def hello()\nend");
    dest.eval("hello()");

    migrate_stateful_values(source, dest);

    // Nothing to test, except to make sure that the above call didn't SIGABRT
    // (which it previously did)
}

void test_migrate_stateful_compound_value()
{
    Branch source;
    Term* l_source = source.eval("state l = []");

    int_value(as_branch(l_source), 1);
    int_value(as_branch(l_source), 2);
    int_value(as_branch(l_source), 3);

    Branch dest;
    Term* l = dest.eval("state l = []");

    test_assert(terms_match_for_migration(l_source, l));

    migrate_stateful_values(source, dest);

    test_assert(is_branch(l_source));
    test_assert(is_branch(l));
    test_assert(as_branch(l).length() == 3);
    test_assert(as_branch(l)[0]->asInt() == 1);
    test_assert(as_branch(l)[1]->asInt() == 2);
    test_assert(as_branch(l)[2]->asInt() == 3);
}

void test_reset_state()
{
    Branch branch;

    Term* i = branch.eval("state i = 5");

    test_assert(i->asInt() == 5);

    as_int(i) = 11;

    test_assert(i->asInt() == 11);

    reset_state(branch);

    evaluate_branch(branch);

    test_assert(i->asInt() == 5);
}

void try_to_migrate_value_that_isnt_allocced()
{
    // This code once caused a crash
    Branch source, dest;
    Term* i = create_stateful_value(source, INT_TYPE, "i");
    as_int(i) = 5;
    dealloc_value(i);

    Term* dest_i = create_stateful_value(dest, INT_TYPE, "i");
    as_int(dest_i) = 4;

    migrate_stateful_values(source, dest);
}

void bug_where_stateful_function_wouldnt_update_inputs()
{
    Branch branch;

    branch.eval("def a() state i end");
    branch.eval("def b(int i) : int; a(); return i; end");

    test_assert(branch);

    Term* x = branch.eval("x = 1");
    Term* b_call = branch.eval("b(x)");

    test_assert(b_call->asInt() == 1);

    x->asInt() = 2;
    evaluate_branch(branch);

    dump_branch(branch);

    test_assert(b_call->asInt() == 2);
}

void register_tests()
{
    REGISTER_TEST_CASE(stateful_code_tests::test_simple);
    REGISTER_TEST_CASE(stateful_code_tests::function_with_hidden_state_term);
    REGISTER_TEST_CASE(stateful_code_tests::subroutine_expansion_during_migrate);
    REGISTER_TEST_CASE(stateful_code_tests::test_load_and_save);
    REGISTER_TEST_CASE(stateful_code_tests::test_get_type_from_branches_stateful_terms);
    REGISTER_TEST_CASE(stateful_code_tests::stateful_value_evaluation);
    REGISTER_TEST_CASE(stateful_code_tests::initialize_from_expression);
    REGISTER_TEST_CASE(stateful_code_tests::one_time_assignment);
    REGISTER_TEST_CASE(stateful_code_tests::one_time_assignment_inside_for_loop);
    REGISTER_TEST_CASE(stateful_code_tests::state_inside_lots_of_nested_functions);
    REGISTER_TEST_CASE(stateful_code_tests::migrate_subroutine_with_no_hidden_state);
    REGISTER_TEST_CASE(stateful_code_tests::test_migrate_stateful_compound_value);
    REGISTER_TEST_CASE(stateful_code_tests::test_reset_state);
    REGISTER_TEST_CASE(stateful_code_tests::try_to_migrate_value_that_isnt_allocced);
    REGISTER_TEST_CASE(stateful_code_tests::bug_where_stateful_function_wouldnt_update_inputs);
}

} // namespace stateful_code_tests

} // namespace circa
