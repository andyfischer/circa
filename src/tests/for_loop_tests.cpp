// Copyright (c) Paul Hodge. See LICENSE file for license terms.

// Unit tests for for-loops

#include "common_headers.h"

#include "circ_internal.h"

namespace circa {
namespace for_loop_tests {

std::vector<int> gSpyResults;

CA_FUNCTION(spy_function)
{
    gSpyResults.push_back(as_int(INPUT(0)));
}

void test_simple()
{
    Branch branch;
    import_function(&branch, spy_function, "spy(int)");
    gSpyResults.clear();

    branch.compile("for i in 0..5 { spy(i) }");

    evaluate_branch(&branch);

    test_assert(gSpyResults.size() == 5);
    test_assert(gSpyResults[0] == 0);
    test_assert(gSpyResults[1] == 1);
    test_assert(gSpyResults[2] == 2);
    test_assert(gSpyResults[3] == 3);
    test_assert(gSpyResults[4] == 4);
}

void type_inference_for_iterator()
{
    Branch branch;
    Term* loop = branch.compile("for i in [1] {}");
    Term* iterator = get_for_loop_iterator(loop);
    //test_assert(iterator->type == INT_TYPE);

    // test a situation where we can't do inference
    loop = branch.compile("for i in [] {}");
    iterator = get_for_loop_iterator(loop);
    test_equals(iterator->type->name, "any");
}

void test_rebind_external()
{
    Branch branch;
    branch.compile("a = 0");
    branch.compile("for i in [1] { a = 1 } a=a");
    evaluate_branch(&branch);
    test_assert(&branch);
    test_assert(branch["a"]->asInt() == 1);
}

void test_rebind_internally()
{
    // The reason we have a=a at the end is because for-loop exports aren't copied back
    // to their respective terms.
    Branch branch;
    branch.compile("a = 0");
    branch.compile("for i in [0 0 0] { a += 1 } a = a");

    evaluate_branch(&branch);
    test_assert(&branch);
    test_equals(branch["a"], "3");

    branch.compile("found_3 = false");
    branch.compile("for n in [5 3 1 9 0] { if n == 3 { found_3 = true } }");
    evaluate_branch(&branch);

    test_assert(branch["found_3"]->asBool());

    branch.compile("found_3 = false");
    branch.compile("for n in [2 4 6 8] { if n == 3 { found_3 = true } }");
    evaluate_branch(&branch);
    test_assert(branch["found_3"]->asBool() == false);
}

void test_rewrite_input_list()
{
    Branch branch;
    branch.compile("l = [1 2 3]");
    branch.compile("for i in @l { i += 1 }");
    evaluate_branch(&branch);

    List* l = List::checkCast(branch["l"]);
    test_assert(l != NULL);
    test_equals(l->toString(), "[2, 3, 4]");
}

void test_state_simple()
{
    Branch branch;
    EvalContext context;

    branch.compile("for i in [1 2 3] { state s = i }");

    evaluate_branch(&context, &branch);
    test_equals(&context.state, "{_for: [{s: 1}, {s: 2}, {s: 3}]}");

    branch.clear();

    EvalContext context2;

    branch.compile("l = [1 2 3]; for i in @l { state s = 0; s += i }");

    evaluate_branch(&context2, &branch);
    test_equals(&context2.state, "{l_1: [{s: 1}, {s: 2}, {s: 3}]}");
    evaluate_branch(&context2, &branch);
    test_equals(&context2.state, "{l_1: [{s: 2}, {s: 4}, {s: 6}]}");
    evaluate_branch(&context2, &branch);
    test_equals(&context2.state, "{l_1: [{s: 3}, {s: 6}, {s: 9}]}");
}

void test_state_nested()
{
    Branch branch;
    EvalContext context;

    branch.compile("for a in [1 2] { for b in [3 4] { for c in [5 6] { state s = c } } }");
    evaluate_branch(&context, &branch);

    test_equals(&context.state, "{_for: [{_for: [{_for: [{s: 5}, {s: 6}]}, "
            "{_for: [{s: 5}, {s: 6}]}]}, {_for: [{_for: [{s: 5}, {s: 6}]}, "
            "{_for: [{s: 5}, {s: 6}]}]}]}");
}

void test_produce_output()
{
    Branch branch;
    branch.compile("x = for i in 0..5 { i + 1 }");
    evaluate_branch(&branch);
    List* x = List::checkCast(branch["x"]);
    test_equals(x->length(), 5);
    test_equals(x->get(0), "1");
    test_equals(x->get(1), "2");
    test_equals(x->get(4), "5");
}

void test_break()
{
    Branch branch;
    internal_debug_function::spy_clear();
    branch.compile("for i in [1 2 3 4] { if i == 3 { break } test_spy(i) }");
    evaluate_branch(&branch);
    test_equals(internal_debug_function::spy_results(), "[1, 2]");
}

void test_nested_break()
{
    Branch branch;

    internal_debug_function::spy_clear();
    branch.compile("for i in ['a' 'b'] "
            "{ for j in [1 2 3] { if j == 2 { break } test_spy([i j]) }}");
    evaluate_branch(&branch);

    test_equals(internal_debug_function::spy_results(), "[['a', 1], ['b', 1]]");
}

void test_continue()
{
    Branch branch;

    internal_debug_function::spy_clear();
    branch.compile("for i in [1 2 3 4] { if i == 3 { continue } test_spy(i) }");
    evaluate_branch(&branch);

    test_equals(internal_debug_function::spy_results(), "[1, 2, 4]");
}

void register_tests()
{
    REGISTER_TEST_CASE(for_loop_tests::test_simple);
    REGISTER_TEST_CASE(for_loop_tests::type_inference_for_iterator);
    REGISTER_TEST_CASE(for_loop_tests::test_rebind_external);
    REGISTER_TEST_CASE(for_loop_tests::test_rebind_internally);
    REGISTER_TEST_CASE(for_loop_tests::test_rewrite_input_list);
    REGISTER_TEST_CASE(for_loop_tests::test_state_simple);
    REGISTER_TEST_CASE(for_loop_tests::test_state_nested);
    REGISTER_TEST_CASE(for_loop_tests::test_produce_output);
    REGISTER_TEST_CASE(for_loop_tests::test_break);
    REGISTER_TEST_CASE(for_loop_tests::test_nested_break);
    REGISTER_TEST_CASE(for_loop_tests::test_continue);
}

} // for_loop_tests

} // namespace circa
