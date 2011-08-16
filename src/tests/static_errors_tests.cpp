// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include <circa.h>

namespace circa {
namespace static_errors_tests {

void input_type_error()
{
    Branch branch;
    Term* t = branch.compile("add_f('hi', 'bye')");

    test_assert(has_static_error(t));
}

void no_error()
{
    Branch branch;
    Term* t = branch.compile("1 + 2");
    test_assert(!has_static_error(t));
}

void strip_error_location(std::string& s)
{
    s = s.substr(s.find_first_of("]") + 2, s.length());
}

void test_unknown_type()
{
    Branch branch;
    branch.compile("type T { X x }");
    Term* t = branch[0];
    test_assert(t->name == "X");
    test_assert(t->function == UNKNOWN_TYPE_FUNC);
    std::string msg = get_static_error_message(t);
    strip_error_location(msg);
    test_equals(msg, "Unknown type: X");
    test_assert(has_static_errors(branch));
}

void test_unknown_identifier()
{
    Branch branch;
    Term* t = branch.eval("charlie");
    std::string msg = get_static_error_message(t);
    strip_error_location(msg);
    test_equals(msg, "Unknown identifier: charlie");
    test_assert(has_static_error(t));
    test_assert(has_static_errors(branch));

    branch.clear();
    t = branch.eval("a:b");
    #if 0
    TEST_DISABLED
    test_equals(get_static_error_message(t), "Unknown identifier: a:b");
    test_assert(has_static_errors(branch));
    #endif
}

void wrong_input_count()
{
    Branch branch;
    branch.compile("def f(int x) {}");

    Term* t = branch.compile("f(1 2)");
    std::string msg = get_static_error_message(t);
    strip_error_location(msg);
    test_equals(msg, "Too many inputs (2), function f expects only 1");

    Term* t2 = branch.compile("f()");
    msg = get_static_error_message(t2);
    strip_error_location(msg);
    test_equals(msg, "Too few inputs (0), function f expects 1");
}

void crash_with_overloaded_varargs()
{
    Branch branch;

    branch.compile("def f() {}");
    branch.compile("def f2(int i) {}");
    branch.compile("g = overloaded_function(f f2)");

    Term* t = branch.compile("g(1)");

    // once caused a crash:
    has_static_error(t);
}

void input_type_mismatch()
{
    Branch branch;
    branch.compile("def f(string) {}");
    Term* t = branch.compile("f(1)");

    std::string msg = get_static_error_message(t);
    strip_error_location(msg);
    test_equals(msg, "Type mismatch for input 0: The input expression has type "
            "int, but function f expects type string");
}

void register_tests()
{
    REGISTER_TEST_CASE(static_errors_tests::input_type_error);
    REGISTER_TEST_CASE(static_errors_tests::no_error);
    REGISTER_TEST_CASE(static_errors_tests::test_unknown_type);
    REGISTER_TEST_CASE(static_errors_tests::test_unknown_identifier);
    REGISTER_TEST_CASE(static_errors_tests::wrong_input_count);
    REGISTER_TEST_CASE(static_errors_tests::crash_with_overloaded_varargs);
    REGISTER_TEST_CASE(static_errors_tests::input_type_mismatch);
}

} // namespace static_errors_tests

} // namespace circa
