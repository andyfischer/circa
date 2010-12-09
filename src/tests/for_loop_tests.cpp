// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

// Unit tests for for-loops

#include "common_headers.h"

#include "circa.h"

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
    import_function(branch, spy_function, "spy(int)");
    gSpyResults.clear();

    branch.compile("for i in 0..5\nspy(i)\nend");

    evaluate_branch(branch);

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
    Term* loop = branch.compile("for i in [1]\nend");
    Term* iterator = get_for_loop_iterator(loop);
    //test_assert(iterator->type == INT_TYPE);

    // test a situation where we can't do inference
    loop = branch.compile("for i in []\nend");
    iterator = get_for_loop_iterator(loop);
    test_assert(iterator->type == ANY_TYPE);
}

void test_rebind_external()
{
    Branch branch;
    branch.compile("a = 0");
    branch.compile("for i in [1]; a = 1; end; a=a");
    evaluate_branch(branch);
    test_assert(branch);
    test_assert(branch["a"]->asInt() == 1);
}

void test_rebind_internally()
{
    // The reason we have a=a at the end is because for-loop exports aren't copied back
    // to their respective terms.
    Branch branch;
    branch.compile("a = 0");
    branch.compile("for i in [0 0 0]; a += 1; end; a = a");
    evaluate_branch(branch);
    test_assert(branch);
    test_assert(branch["a"]->asInt() == 3);

    branch.compile("found_3 = false");
    branch.compile("for n in [5 3 1 9 0]; if n == 3; found_3 = true; end; end; found_3=found_3");
    evaluate_branch(branch);

    test_assert(branch["found_3"]->asBool());

    branch.compile("found_3 = false");
    branch.compile("for n in [2 4 6 8]; if n == 3; found_3 = true; end; end; found_3=found_3");
    evaluate_branch(branch);
    test_assert(branch["found_3"]->asBool() == false);
}

void test_rewrite_input_list()
{
    Branch branch;
    branch.compile("l = [1 2 3]");
    branch.compile("for i in @l; i += 1; end");
    evaluate_branch(branch);

    List* l = List::checkCast(branch["l"]);
    test_assert(l != NULL);
    test_equals(l->toString(), "[2, 3, 4]");
}

void test_state_simple()
{
    Branch branch;
    EvalContext context;

    branch.compile("for i in [1 2 3]; state s = i; end");

    evaluate_branch(&context, branch);
    test_equals(&context.state, "[_for: [[s: 1], [s: 2], [s: 3]]]");

    branch.clear();
    context = EvalContext();

    branch.compile("l = [1 2 3]; for i in @l; state s = 0; s += i; end");

    evaluate_branch(&context, branch);
    test_equals(&context.state, "[l_1: [[s: 1], [s: 2], [s: 3]]]");
    evaluate_branch(&context, branch);
    test_equals(&context.state, "[l_1: [[s: 2], [s: 4], [s: 6]]]");
    evaluate_branch(&context, branch);
    test_equals(&context.state, "[l_1: [[s: 3], [s: 6], [s: 9]]]");
}

void test_state_nested()
{
    Branch branch;
    EvalContext context;

    branch.compile("for a in [1 2] for b in [3 4] for c in [5 6] state s = c end end end");
    evaluate_branch(&context, branch);

    test_equals(&context.state, "[_for: [[_for: [[_for: [[s: 5], [s: 6]]], "
            "[_for: [[s: 5], [s: 6]]]]], [_for: [[_for: [[s: 5], [s: 6]]], "
            "[_for: [[s: 5], [s: 6]]]]]]]");
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
}

} // for_loop_tests

} // namespace circa
