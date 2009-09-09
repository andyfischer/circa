// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

// Unit tests for for-loops

#include "common_headers.h"

#include "circa.h"

namespace circa {
namespace for_loop_tests {

std::vector<int> gSpyResults;

void spy_function(Term* caller)
{
    gSpyResults.push_back(as_int(caller->input(0)));
}

void test_simple()
{
    Branch branch;
    import_function(branch, spy_function, "spy(int)");
    gSpyResults.clear();

    branch.compile("for i in range(5)\nspy(i)\nend");

    evaluate_branch(branch);

    test_assert(gSpyResults.size() == 5);
    test_assert(gSpyResults[0] == 0);
    test_assert(gSpyResults[1] == 1);
    test_assert(gSpyResults[2] == 2);
    test_assert(gSpyResults[3] == 3);
    test_assert(gSpyResults[4] == 4);
}

void test_state()
{
    Branch branch;

    // Make sure that each iteration of the loop has a different copy of stateful values
    Term* loop = branch.compile("for i in [1 2 3]\nstate s = unique_id()\nend");

    evaluate_term(loop);

    Term* s_iter0 = get_for_loop_iteration_state(loop, 0)["s"];
    Term* s_iter1 = get_for_loop_iteration_state(loop, 1)["s"];
    Term* s_iter2 = get_for_loop_iteration_state(loop, 2)["s"];
    test_assert(s_iter0 != NULL);
    test_assert(s_iter1 != NULL);
    test_assert(s_iter2 != NULL);
    test_assert(as_int(s_iter0) != as_int(s_iter1));
    test_assert(as_int(s_iter1) != as_int(s_iter2));
    test_assert(as_int(s_iter0) != as_int(s_iter2));
}

void type_inference_for_iterator()
{
    Branch branch;
    Term* loop = branch.compile("for i in [1]\nend");
    Term* iterator = get_for_loop_iterator(loop);
    test_assert(iterator->type == INT_TYPE);

    // test a situation where we can't do inference
    loop = branch.compile("for i in []\nend");
    iterator = get_for_loop_iterator(loop);
    test_assert(iterator->type == ANY_TYPE);

    // test a situation where inference is required to parse the inner branch
    loop = branch.compile("for i in [1]\ni = i * 2\nend");
    Branch& inner_code = get_for_loop_code(loop);
    test_assert(inner_code["i"]->function->name == "mult_i");

    // test that we find a common type
    loop = branch.compile("for i in [1 1.0]\nend");
    iterator = get_for_loop_iterator(loop);
    test_assert(iterator->type == FLOAT_TYPE);

    loop = branch.compile("for i in [1 1.0 'string']\nend");
    iterator = get_for_loop_iterator(loop);
    test_assert(iterator->type == ANY_TYPE);
}

void test_rebind_external()
{
    Branch branch;
    branch.eval("a = 0");
    branch.eval("for i in [1]; a = 1; end");
    test_assert(branch);
    test_assert(branch["a"]->asInt() == 1);

}

void test_rebind_internally()
{
    Branch branch;
    branch.eval("a = 0");
    branch.eval("for i in [0 0 0]; a += 1; end");
    test_assert(branch);
    test_assert(branch["a"]->asInt() == 3);

    branch.eval("found_3 = false");
    branch.eval("for n in [5 3 1 9 0]; if n == 3; found_3 = true; end; end");
    test_assert(branch["found_3"]->asBool());

    branch.eval("found_3 = false");
    branch.eval("for n in [2 4 6 8]; if n == 3; found_3 = true; end; end");
    test_assert(branch["found_3"]->asBool() == false);
}

void register_tests()
{
    REGISTER_TEST_CASE(for_loop_tests::test_simple);
    REGISTER_TEST_CASE(for_loop_tests::type_inference_for_iterator);
    REGISTER_TEST_CASE(for_loop_tests::test_rebind_external);
    REGISTER_TEST_CASE(for_loop_tests::test_rebind_internally);
}

} // for_loop_tests

} // namespace circa
