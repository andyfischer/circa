// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "inspection.h"
#include "kernel.h"

namespace path_expression_test {

void simple_name_match()
{
    Block block;
    block.compile("a = 1; b = 2; c = 3");

    Term* a = find_term_from_path(&block, "a");
    Term* b = find_term_from_path(&block, "b");
    Term* c = find_term_from_path(&block, "c");
    test_assert(a != NULL);
    test_assert(b != NULL);
    test_assert(c != NULL);
    test_equals(term_value(a), "1");
    test_equals(term_value(b), "2");
    test_equals(term_value(c), "3");
}

void nested_name_match()
{
    Block block;
    Term* a = block.compile("a = section { b = section { c = 4 }}");
    block.compile("a2 = section { b2 = 5 }");

    test_assert(a == find_term_from_path(&block, "a"));
    Term* c = find_term_from_path(&block, "a/b/c");
    Term* b2 = find_term_from_path(&block, "a2/b2");

    test_assert(c != NULL);
    test_assert(b2 != NULL);
    test_equals(term_value(c), "4");
    test_equals(term_value(b2), "5");
}

void wildcard_nested_match()
{
    Block block;
    block.compile("a = section { b = section { c = 5 } }");
    block.compile("a = section { b2 = section { d = 6 } }");

    Term* d = find_term_from_path(&block, "a/*/d");
    test_assert(d != NULL);
    test_equals(term_value(d), "6");
}

void recursive_wildcard_match()
{
    Block block;
    block.compile("a = 1");
    block.compile("b = section { c = 2 }");
    block.compile("d = section { e = section { f = 3 } }");

    Term* a = find_term_from_path(&block, "**/a");
    Term* c = find_term_from_path(&block, "**/c");
    Term* f = find_term_from_path(&block, "** / f");
    test_assert(a != NULL);
    test_assert(c != NULL);
    test_assert(f != NULL);
    test_equals(term_value(a), "1");
    test_equals(term_value(c), "2");
    test_equals(term_value(f), "3");
}

void function_match()
{
    Block block;
    block.compile("a = add(1 2)");
    block.compile("b = mult(3 4)");

    Term* a = find_term_from_path(&block, "function=add");
    Term* b = find_term_from_path(&block, "function=mult");
    test_assert(a != NULL);
    test_assert(b != NULL);
    test_assert(a->function == FUNCS.add);
    test_assert(b->function == FUNCS.mult);
}

void recursive_function_match()
{
    Block block;
    block.compile("def f() { for i in [1] { if i == 2 { return 3 } } }");

    Term* returnCall = find_term_from_path(&block, "** / function=return");
    test_assert(returnCall != NULL);
    test_assert(returnCall->function == FUNCS.return_func);
}

void register_tests()
{
    REGISTER_TEST_CASE(path_expression_test::simple_name_match);
    REGISTER_TEST_CASE(path_expression_test::nested_name_match);
    REGISTER_TEST_CASE(path_expression_test::wildcard_nested_match);
    REGISTER_TEST_CASE(path_expression_test::recursive_wildcard_match);
    REGISTER_TEST_CASE(path_expression_test::function_match);
    REGISTER_TEST_CASE(path_expression_test::recursive_function_match);
}

}
