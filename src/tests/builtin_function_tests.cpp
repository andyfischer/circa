// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include <circa_internal.h>
#include "importing_macros.h"

namespace circa {
namespace builtin_function_tests {

void test_int()
{
    Branch branch;

    test_assert(INT_T.formatSource != NULL);

    Term* four = create_int(&branch, 4);
    Term* another_four = create_int(&branch, 4);
    Term* five = create_int(&branch, 5);

    test_assert(equals(four, another_four));
    test_assert(!equals(four, five));

    test_equals(four->toString(), "4");
}

void test_float()
{
    Branch branch;

    Type* floatType = &FLOAT_T;

    test_assert(floatType->equals != NULL);
    test_assert(floatType->formatSource != NULL);

    Term* point_one = create_float(&branch, .1f);
    Term* point_one_again = create_float(&branch, .1f);
    Term* point_two = create_float(&branch, 0.2f);

    test_assert(equals(point_one, point_one_again));
    test_assert(equals(point_two, point_two));

    test_equals(point_one->toString(), "0.1");
}

void test_do_once()
{
    Branch branch;
    EvalContext context;

    Term* x = branch.compile("x = 1");
    Term* t = branch.compile("do_once()");
    nested_contents(t)->compile("unsafe_assign(x,2)");

    test_assert(&branch);

    test_assert(as_int(x) == 1);

    // the assign() inside do_once should modify x
    evaluate_branch(&context, &branch);
    test_assert(as_int(x) == 2);

    // but if we call it again, it shouldn't do that any more
    set_int(x, 3);
    evaluate_branch(&context, &branch);
    test_assert(as_int(x) == 3);
}

void test_changed()
{
    Branch branch;
    EvalContext context;
    Term* x = branch.compile("x = 5");
    Term* changed = branch.compile("changed(x)");

    evaluate_branch(&context, &branch);
    test_assert(changed->asBool() == true);

    evaluate_branch(&context, &branch);
    test_assert(changed->asBool() == false);
    evaluate_branch(&context, &branch);
    test_assert(changed->asBool() == false);

    set_int(x, 6);
    evaluate_branch(&context, &branch);
    test_assert(changed->asBool() == true);
    evaluate_branch(&context, &branch);
    test_assert(changed->asBool() == false);
}

void test_delta()
{
    Branch branch;

    Term* i = branch.compile("i = 0");
    Term* delta = branch.compile("delta(i)");

    EvalContext context;
    evaluate_branch(&context, &branch);

    test_assert(is_float(delta));
    test_equals(delta->toFloat(), 0);
    
    set_int(i, 5);
    evaluate_branch(&context, &branch);
    test_assert(is_float(delta));
    test_equals(delta->toFloat(), 5);

    // do another evaluation without changing i, delta is now 0
    evaluate_branch(&context, &branch);
    test_equals(delta->toFloat(), 0);

    set_int(i, 2);
    evaluate_branch(&context, &branch);
    test_equals(delta->toFloat(), -3);

    set_int(i, 0);
    evaluate_branch(&context, &branch);
    test_equals(delta->toFloat(), -2);
}

void test_message_passing()
{
    Branch branch;
    EvalContext context;
    Term* inbox = branch.compile("inbox = receive('inbox_name')");
    Term* send = branch.compile("send('inbox_name', 1)");

    // Before running, message queue should be empty
    test_assert(inbox->numElements() == 0);
    test_equals(&context.messages, "{}");

    // First run, inbox is still empty, but there is 1 message in transit
    evaluate_branch(&context, &branch);
    test_assert(inbox->numElements() == 0);
    test_equals(&context.messages, "{inbox_name: [1]}");

    // Second run, inbox now returns 1
    evaluate_branch(&context, &branch);
    test_assert(inbox->numElements() == 1);
    test_assert(inbox->getIndex(0)->asInt() == 1);
    test_equals(&context.messages, "{inbox_name: [1]}");

    // Delete the send() call
    remove_term(send);

    // Third run, inbox still returns 1 (from previous call), message queue is empty
    evaluate_branch(&context, &branch);
    test_assert(inbox->numElements() == 1);
    test_assert(inbox->getIndex(0)->asInt() == 1);
    test_equals(&context.messages, "{inbox_name: []}");

    // Fourth run, inbox is empty again
    evaluate_branch(&context, &branch);
    test_assert(inbox->numElements() == 0);
    test_equals(&context.messages, "{inbox_name: []}");
}

void test_message_passing2()
{
    // Repro a bug from plastic's runtime
    Branch branch;
    branch.compile(
        "state last_output = 1\n"
        "incoming = receive('inbox_name')\n"
        "def send_func(any s)\n"
        "  send('inbox_name', s)\n"
        "for s in incoming\n"
        "  last_output = s\n"
        "send_func(2)\n");

    EvalContext context;
    evaluate_branch(&context, &branch);

    test_equals(&context.state, "{last_output: 1}");

    evaluate_branch(&context, &branch);
    test_equals(&context.state, "{last_output: 2}");

    evaluate_branch(&context, &branch);
    test_equals(&context.state, "{last_output: 2}");

    evaluate_branch(&context, &branch);
    test_equals(&context.state, "{last_output: 2}");
}

void test_run_single_statement()
{
    Branch branch;
    branch.eval("br = { test_spy(1) test_spy('two') test_spy(3) \n"
        "-- this is a comment \n"
        "\n"
        "test_spy(4) }");

    internal_debug_function::spy_clear();
    branch.eval("run_single_statement(br, 0)");
    test_equals(internal_debug_function::spy_results(), "[1]");

    internal_debug_function::spy_clear();
    branch.eval("run_single_statement(br, 1)");
    test_equals(internal_debug_function::spy_results(), "['two']");

    internal_debug_function::spy_clear();
    branch.eval("run_single_statement(br, 2)");
    test_equals(internal_debug_function::spy_results(), "[3]");

    internal_debug_function::spy_clear();
    branch.eval("run_single_statement(br, 3)");
    test_equals(internal_debug_function::spy_results(), "[4]");
}

void register_tests()
{
    REGISTER_TEST_CASE(builtin_function_tests::test_int);
    REGISTER_TEST_CASE(builtin_function_tests::test_float);
    REGISTER_TEST_CASE(builtin_function_tests::test_cond_with_int_and_float);
    //TEST_DISABLED REGISTER_TEST_CASE(builtin_function_tests::test_do_once);
    //TEST_DISABLED REGISTER_TEST_CASE(builtin_function_tests::test_changed);
    //TEST_DISABLED REGISTER_TEST_CASE(builtin_function_tests::test_delta);
    //TEST_DISABLED REGISTER_TEST_CASE(builtin_function_tests::test_message_passing);
    //TEST_DISABLED REGISTER_TEST_CASE(builtin_function_tests::test_message_passing2);
    //TEST_DISABLED REGISTER_TEST_CASE(builtin_function_tests::test_run_single_statement);
}

} // namespace builtin_function_tests

} // namespace circa
