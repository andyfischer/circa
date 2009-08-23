// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include <circa.h>

namespace circa {
namespace source_repro_tests {

// "generate" tests. In these tests, we create source text from code that wasn't
// originally produced by the parser, to make sure it still looks sane.

void generate_source_for_function_calls() {
    Branch branch;

    Term* a = int_value(branch, 5, "a");
    Term* b = int_value(branch, 9, "b");
    Term* c = apply(branch, "add", RefList(a,b));

    test_assert(should_print_term_source_line(a));
    test_assert(should_print_term_source_line(b));
    test_assert(should_print_term_source_line(c));
    test_equals(get_branch_source(branch), "a = 5\nb = 9\nadd(a, b)");

    // Same test with anonymous values
    branch.clear();
    Term* d = int_value(branch, 3);
    Term* e = int_value(branch, 4);
    /*Term* f =*/ apply(branch, "add", RefList(d,e));

    /*
    TODO, fix this
    test_assert(!should_print_term_source_line(d));
    test_assert(!should_print_term_source_line(e));
    test_assert(should_print_term_source_line(f));
    test_equals(get_branch_source(branch), "add(3, 4)");
    
    // Do a test where some calls are parser-created, and then user-created calls
    // are added.
    branch.clear();
    branch.compile("a = 1");
    branch.compile("b = 2");
    a = int_value(branch, 3, "c");
    b = int_value(branch, 4, "d");
    apply(branch, "add", RefList(a,b));

    test_equals(get_branch_source(branch), "a = 1\nb = 2\nc = 3\nd = 4\nadd(c, d)");
    */
}

void generate_source_for_literal_list() {
    Branch branch;
    
    Branch& a = create_list(branch, "a");

    test_equals(get_branch_source(branch), "a = []");

    int_value(a, 5);
    test_equals(get_branch_source(branch), "a = [5]");

    int_value(a, 100);
    test_equals(get_branch_source(branch), "a = [5,100]");

/*
    branch.clear();
    Term* apple = int_value(branch, 12, "apple");
    create_list(&branch, "b").append(apple);
    test_equals(get_branch_source(branch), "apple = 12\nb = [apple]");
*/
}

void bug_reproducing_list_after_eval()
{
    // There once was a bug where source repro would fail when using a list
    // in a vectorized function, but only after that piece of code had been
    // evaluated. This was because vectorize_vv was calling apply() which
    // was changing the 'statement' property of its inputs.
    Branch branch;

    Term* sum = branch.compile("[1 1] + [1 1]");
    Term* in0 = sum->input(0);
    Term* in1 = sum->input(0);

    test_equals(get_term_source(in0), "[1 1]");
    test_equals(get_term_source(in1), "[1 1]");
    test_equals(get_source_of_input(sum, 0), "[1 1] ");
    test_equals(get_source_of_input(sum, 1), " [1 1]");
    test_equals(get_branch_source(branch), "[1 1] + [1 1]");

    evaluate_branch(branch);

    test_equals(get_term_source(in0), "[1 1]");
    test_equals(get_term_source(in1), "[1 1]");
    test_equals(get_source_of_input(sum, 0), "[1 1] ");
    test_equals(get_source_of_input(sum, 1), " [1 1]");

    test_equals(get_branch_source(branch), "[1 1] + [1 1]");
}

void register_tests() {
    REGISTER_TEST_CASE(source_repro_tests::generate_source_for_function_calls);
    REGISTER_TEST_CASE(source_repro_tests::generate_source_for_literal_list);
    REGISTER_TEST_CASE(source_repro_tests::bug_reproducing_list_after_eval);
}

} // namespace source_repro_tests
} // namespace circa
