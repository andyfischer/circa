// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "circa/circa.h"
#include "term.h"

using namespace circa;

void test_assert_function(bool condition, int line, const char* file);
void test_assert_function(Term* term, int line, const char* file);
void test_assert_function(Block* block, int line, const char* file);
void test_assert_function(Stack* stack, int line, const char* file);
void test_fail_function(int line, const char* file);
void test_equals_function(TermList const& a, TermList const& b,
        const char* aText, const char* bText, int line, const char* file);
void test_equals_function(float a, float b,
        const char* aText, const char* bText,
        int line, const char* file);
void test_equals_function(std::string a, std::string b,
        const char* aText, const char* bText,
        int line, const char* file);
void test_equals_function(Value* a, std::string b,
        const char* aText, const char* bText,
        int line, const char* file);
void test_equals_function(Value* a, float b,
        const char* aText, const char* bText,
        int line, const char* file);

#define test_assert(c) test_assert_function((c), __LINE__, __FILE__)
#define test_fail() test_fail_function(__LINE__, __FILE__)
#define test_equals(a,b) test_equals_function(a,b,#a,#b,__LINE__,__FILE__)

// If 'block' has a static error, then print something to stdout, and declare
// the current test failed, and return true. If this returns false then there
// was no error.
bool test_fail_on_static_error(Block* block);

// If context has recorded a runtime error, then print something to stdout, and
// declare the current test failed, and return true. If this returns false then
// there was no error.
bool test_fail_on_runtime_error(Stack& context);

Value* temp_string(const char* str);
World* test_world();
void test_write_fake_file(const char* filename, int version, const char* contents);

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

bool run_tests(std::string const& searchStr);

// Run all unit tests, returns true if all passed.
bool run_all_tests();

std::vector<std::string> list_all_test_names();
std::string get_current_test_name();
void declare_current_test_failed();
bool current_test_has_failed();
