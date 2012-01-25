// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include <circa.h>

namespace circa {
namespace stateful_code_snippets {

void test_snippet(std::string const& source)
{
    Branch branch;
    branch.compile(source);

    if (test_fail_on_static_error(&branch))
        return;

    EvalContext context;
    evaluate_branch(&context, &branch);

    if (test_fail_on_runtime_error(context))
        return;

    // Try stripping orphaned state, this should not have an effect.
    TValue trash;
    strip_orphaned_state(&branch, &context.state, &trash);

    if (!is_null(&trash)) {
        std::cout << "Falsely orphaned state in " << get_current_test_name() << std::endl;
        std::cout << "Code = " << source << std::endl;
        std::cout << "Trash = " << trash.toString() << std::endl;
        declare_current_test_failed();
        return;
    }
}

void test_trimmed_state(std::string const& source, std::string const& dest,
    std::string const& expectedTrash)
{
    Branch sourceBranch;
    sourceBranch.compile(source);

    if (test_fail_on_static_error(&sourceBranch))
        return;

    EvalContext context;
    evaluate_branch(&context, &sourceBranch);

    if (test_fail_on_runtime_error(context))
        return;

    Branch destBranch;
    destBranch.compile(dest);

    if (test_fail_on_static_error(&destBranch))
        return;

    TValue trash;
    strip_orphaned_state(&destBranch, &context.state, &trash);

    if (expectedTrash != trash.toString()) {
        declare_current_test_failed();
        std::cout << "In test " << get_current_test_name() << std::endl;
        std::cout << expectedTrash << " != " << trash.toString() << std::endl;
    }
}

void simple_tests()
{
    test_snippet("state s");
    test_snippet("state int s");
    test_snippet("state s = 0");
    test_snippet("state int s = 0");

    test_trimmed_state("state s = 0", "", "{s: 0}");
    test_trimmed_state("state int s; state string t", "", "{s: 0, t: ''}");
    test_trimmed_state("state int s; state string t", "state s", "{t: ''}");
}

void name_overlapping()
{
    test_snippet("state int apple = 0");
    test_snippet("state string apple = 'hi'");
    test_snippet("state bool apple = false");

    test_trimmed_state("state s = 0", "", "{s: 0}");
}

void if_blocks()
{
    test_snippet("if true { state a }");
    test_snippet("if true { state a } else { state b }");

    test_trimmed_state("if true { state a }", "", "{_if_block: [{a: null}]}");
}

void function_calls()
{
    test_snippet("def f() { state a }; f() f() f()");
    test_snippet("def f() { state a }; def g() { f() } g() g()");

    test_trimmed_state("for i in [1 2 3] { state x = i }", "",
            "{_for: [{x: 1}, {x: 2}, {x: 3}]}");
}

void for_loops()
{
    test_snippet("for i in [1] {}");
    test_snippet("for i in [1] { state s }");
    test_snippet("for i in [1 2 3] { state s }");

    test_trimmed_state("for i in [1 2 3] { state s }", "", "{_for: [{s: null}, {s: null}, {s: null}]}");
}

void register_tests()
{
    REGISTER_TEST_CASE(stateful_code_snippets::simple_tests);
    REGISTER_TEST_CASE(stateful_code_snippets::name_overlapping);
    REGISTER_TEST_CASE(stateful_code_snippets::if_blocks);
    REGISTER_TEST_CASE(stateful_code_snippets::function_calls);
    REGISTER_TEST_CASE(stateful_code_snippets::for_loops);
}

} // stateful_code_snippets
} // namespace circa
