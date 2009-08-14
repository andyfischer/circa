// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

// testing.cpp : A mini-framework for running unit tests.

#ifndef CIRCA_TESTING_INCLUDED
#define CIRCA_TESTING_INCLUDED

#include "ref_list.h"
#include "term.h"

namespace circa {

void _test_assert_function(bool condition, int line, const char* file);
void _test_assert_function(Term* term, int line, const char* file);
void _test_assert_function(Branch& branch, int line, const char* file);
void _test_fail_function(int line, const char* file);
void _test_equals_function(RefList const& a, RefList const& b,
        const char* aText, const char* bText, int line, const char* file);
void _test_equals_function(float a, float b,
        const char* aText, const char* bText,
        int line, const char* file);
void _test_equals_function(std::string a, std::string b,
        const char* aText, const char* bText,
        int line, const char* file);

#define test_assert(c) _test_assert_function((c), __LINE__, __FILE__)
#define test_fail() _test_fail_function( __LINE__, __FILE__)
#define test_equals(a,b) _test_equals_function(a,b,#a,#b,__LINE__,__FILE__)

struct TestCase {
    typedef void (*TestExecuteFunction)();

    std::string name;
    TestExecuteFunction execute;
    bool failed;

    TestCase() : execute(NULL), failed(false) { }

    TestCase(std::string const& _name, TestExecuteFunction _execute)
      : name(_name), execute(_execute), failed(false) { }
};

extern std::vector<TestCase> gTestCases;

#define REGISTER_TEST_CASE(f) gTestCases.push_back(TestCase(#f,f))

void run_test_named(std::string const& testName);
void run_all_tests();
std::vector<std::string> list_all_test_names();
std::string get_current_test_name();
void declare_current_test_failed();

} // namespace circa

#endif
