// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "evaluation.h"
#include "heap_debugging.h"
#include "introspection.h"
#include "kernel.h"
#include "source_repro.h"
#include "static_checking.h"
#include "names.h"
#include "parser.h"
#include "tagged_value.h"
#include "term.h"
#include "type.h"

#include "framework.h"

namespace circa {

std::vector<TestCase> gTestCases;

TestCase gCurrentTestCase;

#ifdef CIRCA_TEST_BUILD

// this function is defined in register_all_tests.cpp
void register_all_tests();

#endif

void post_test_sanity_check();

void test_assert_function(bool condition, int line, const char* file)
{
    if (!condition) {
        std::cout << "Assert failure in " << file << ", line " << line << std::endl;
        declare_current_test_failed();
    }
}

void test_assert_function(Term* term, int line, const char* file)
{
    if (term == NULL) {
        std::cout << "NULL term in " << file << ", line " << line << std::endl;
        declare_current_test_failed();
        return;
    }

    if (has_static_error(term)) {
        std::cout << "Compile error on term " << global_id(term) << std::endl;
        print_static_error(term, std::cout);
        std::cout << std::endl;
        std::cout << "Occurred in " << file << ", line " << line << std::endl;
        declare_current_test_failed();
    }

    if (is_bool(term_value(term)) && !as_bool(term_value(term))) {
        std::cout << "Assertion failed: " << get_term_source_text(term) << std::endl;
        std::cout << "Occurred in " << file << ", line " << line << std::endl;
        declare_current_test_failed();
    }
}

void test_assert_function(Branch* branch, int line, const char* file)
{
    if (!branch_check_invariants_print_result(branch, std::cout)) {
        std::cout << "Branch failed invariant check in " << file << ", line " << line << std::endl;
        declare_current_test_failed();
    }

    List errors;
    check_for_static_errors(&errors, branch);
    if (!errors.empty()) {
        std::cout << "Branch has static errors at " << file << ", line " << line << std::endl;
        print_static_errors_formatted(&errors, std::cout);
        declare_current_test_failed();
    }
}

void test_assert_function(Stack& context, int line, const char* file)
{
    if (context.errorOccurred) {
        std::cout << "Runtime error at " << file << ", line " << line << std::endl;
        print_error_stack(&context, std::cout);
        declare_current_test_failed();
    }
}

void test_fail_function(int line, const char* file)
{
    std::cout << "Test fail in " << file << ", line " << line << std::endl;
    declare_current_test_failed();
}

void test_equals_function(TermList const& a, TermList const& b,
        const char* aText, const char* bText, int line, const char* file)
{
    if (a.length() != b.length()) {
        std::cout << "List equality fail in " << file << ", line " << line << std::endl;
        std::cout << "  " << aText << " has " << a.length() << " items, ";
        std::cout << bText << " has " << b.length() << " items." << std::endl;
        declare_current_test_failed();
        return;
    }

    for (int i=0; i < a.length(); i++) {
        if (a[i] != b[i]) {
            std::cout << "List equality fail in " << file << ", line " << line << std::endl;
            std::cout << "  " << aText << " != " << bText
                << " (index " << i << " differs)" << std::endl;
            declare_current_test_failed();
            return;
        }
    }
}

void test_equals_function(float a, float b,
        const char* aText, const char* bText,
        int line, const char* file)
{
    const float EPSILON = 0.0001f;

    if (fabs(a-b) > EPSILON) {
        std::cout << "Equality fail in " << file << ", line " << line << std::endl;
        std::cout << aText << " [" << a << "] != " << bText << " [" << b << "]" << std::endl;
        declare_current_test_failed();
        return;
    }
}

void test_equals_function(std::string a, std::string b,
        const char* aText, const char* bText,
        int line, const char* file)
{
    if (a != b) {
        std::cout << "Failed assert: " << a << " != " << b;
        std::cout << ", in " << file << ", line " << line << std::endl;
        declare_current_test_failed();
        return;
    }
}

void test_equals_function(caValue* a, std::string b,
        const char* aText, const char* bText,
        int line, const char* file)
{
    return test_equals_function(is_string(a) ? as_string(a) : to_string(a),
            b, aText, bText, line, file);
}

void test_equals_function(caValue* a, float b,
        const char* aText, const char* bText,
        int line, const char* file)
{
    return test_equals_function(to_float(a), b, aText, bText, line, file);
}

bool test_fail_on_static_error(Branch* branch)
{
    if (has_static_errors(branch)) {
        std::cout << "Static error in " << get_current_test_name() << std::endl;
        print_static_errors_formatted(branch, std::cout);
        std::cout << std::endl;
        declare_current_test_failed();
        return true;
    }
    return false;
}

bool test_fail_on_runtime_error(Stack& context)
{
    if (context.errorOccurred) {
        std::cout << "Runtime error in " << get_current_test_name() << std::endl;
        print_error_stack(&context, std::cout);
        std::cout << std::endl;
        declare_current_test_failed();
        return true;
    }
    return false;
}

bool run_test(TestCase& testCase, bool catch_exceptions)
{
    gCurrentTestCase = testCase;

    if (catch_exceptions) {
        try {
            gCurrentTestCase = testCase;
            testCase.execute();

            // the test code may declare itself failed
            bool result = !gCurrentTestCase.failed;

            post_test_sanity_check();
            return result;
        }
        catch (std::runtime_error const& err) {
            std::cout << "Error white running test case " << testCase.name << std::endl;
            std::cout << err.what() << std::endl;
            return false;
        }
    } else {
        testCase.execute();
    }

    post_test_sanity_check();

    return !gCurrentTestCase.failed;
}

bool run_tests(std::string const& searchStr)
{
#ifdef CIRCA_TEST_BUILD
    register_all_tests();
#endif

    int totalTestCount = 0;
    int successCount = 0;
    int failureCount = 0;
    std::vector<TestCase>::iterator it;
    for (it = gTestCases.begin(); it != gTestCases.end(); ++it) {
        if (it->name.find(searchStr) == std::string::npos)
            continue;
        totalTestCount++;
        std::cout << "Running " << it->name << std::endl;
        bool result = run_test(*it, false);
        if (result) successCount++;
        else {
            failureCount++;
            std::cout << "Test failed: " << it->name << std::endl;
        }
    }

    std::cout << "Ran " << totalTestCount << " tests. ";

    if (failureCount == 0) {
        std::cout << "All tests passed." << std::endl;
    } else {
        std::string tests = failureCount == 1 ? "test" : "tests";
        std::cout << failureCount << " " << tests << " failed." << std::endl;
    }

    return failureCount == 0;
}

bool run_all_tests()
{
#ifdef CIRCA_TEST_BUILD
    register_all_tests();
#endif

    int totalTestCount = 0;
    int successCount = 0;
    int failureCount = 0;
    std::vector<TestCase>::iterator it;
    for (it = gTestCases.begin(); it != gTestCases.end(); ++it) {
        totalTestCount++;
        bool result = run_test(*it, false);
        if (result) successCount++;
        else {
            failureCount++;
            std::cout << "Test failed: " << it->name << std::endl;
        }
    }

    std::cout << "Ran " << totalTestCount << " tests. ";

    if (failureCount == 0) {
        std::cout << "All tests passed." << std::endl;
    } else {
        std::string tests = failureCount == 1 ? "test" : "tests";
        std::cout << failureCount << " " << tests << " failed." << std::endl;
    }

    return failureCount == 0;
}

void post_test_sanity_check()
{
    // this once did something
}

std::vector<std::string> list_all_test_names()
{
    std::vector<std::string> output;

#ifdef CIRCA_TEST_BUILD
    register_all_tests();
#endif

    std::vector<TestCase>::iterator it;
    for (it = gTestCases.begin(); it != gTestCases.end(); ++it)
        output.push_back(it->name);

    return output;
}

std::string get_current_test_name()
{
    return gCurrentTestCase.name;
}

void declare_current_test_failed()
{
    gCurrentTestCase.failed = true;
}

bool current_test_has_failed()
{
    return gCurrentTestCase.failed;
}

void test_branch_as_assertions_list(Branch* branch, std::string const& contextStr)
{
    if (has_static_errors(branch)) {
        std::cout << "Static error " << contextStr << ":" << std::endl;
        print_static_errors_formatted(branch, std::cout);
        declare_current_test_failed();
        return;
    }

    std::stringstream checkInvariantsOutput;
    if (!branch_check_invariants_print_result(branch, checkInvariantsOutput)) {
        std::cout << "Failed invariant " << contextStr << std::endl;
        std::cout << checkInvariantsOutput.str() << std::endl;
        declare_current_test_failed();
        return;
    }

    Stack context;
    evaluate_branch(&context, branch);

    if (context.errorOccurred) {
        std::cout << "Runtime error " << contextStr << std::endl;
        print_error_stack(&context, std::cout);
        declare_current_test_failed();
        return;
    }

    int boolean_statements_found = 0;
    for (int i=0; i < branch->length(); i++) {
        Term* term = branch->get(i);
        if (!is_statement(term))
            continue;

        if (!is_bool(term_value(term)))
            continue;

        boolean_statements_found++;

        if (!as_bool(term_value(term))) {
            std::cout << "Assertion failed " << contextStr << std::endl;
            std::cout << "failed: " << get_term_source_text(term) << std::endl;
            declare_current_test_failed();
            return;
        }
    }

    if (boolean_statements_found == 0) {
        std::cout << "No boolean statements found " << contextStr << std::endl;
        declare_current_test_failed();
        return;
    }
}

}
