// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "hashtable.h"
#include "inspection.h"
#include "kernel.h"
#include "static_checking.h"
#include "string_type.h"
#include "world.h"

std::vector<TestCase> gTestCases;
TestCase gCurrentTestCase;

// gTempValue is used for the temp_string convenience function.
Value* gTempValue = NULL;

Value* gFakeFileMap = NULL;

void after_each_test();

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
        Value str;
        format_static_error(term, &str);
        std::cout << as_cstring(&str) << std::endl;
        std::cout << "Occurred in " << file << ", line " << line << std::endl;
        declare_current_test_failed();
    }

    if (is_bool(term_value(term)) && !as_bool(term_value(term))) {
        std::cout << "Assertion failed: " << term->id << std::endl;
        std::cout << "Occurred in " << file << ", line " << line << std::endl;
        declare_current_test_failed();
    }
}

void test_assert_function(Block* block, int line, const char* file)
{
    Value str;
    if (!block_check_invariants_print_result(block, &str)) {
        std::cout << as_cstring(&str) << std::endl;
        std::cout << "Block failed invariant check in " << file << ", line " << line << std::endl;
        declare_current_test_failed();
    }

    Value errors;
    check_for_static_errors(&errors, block);
    if (!errors.isEmpty()) {
        std::cout << "Block has static errors at " << file << ", line " << line << std::endl;
        print_static_errors_formatted(&errors, &str);
        std::cout << as_cstring(&str) << std::endl;
        declare_current_test_failed();
    }
}

void test_assert_function(VM* vm, int line, const char* file)
{
    if (vm_has_error(vm)) {
        std::cout << "VM error caught at " << file << ", line " << line << std::endl;
        //circa_dump_stack_trace(vm);
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

void test_equals_function(Value* a, std::string b,
        const char* aText, const char* bText,
        int line, const char* file)
{
    Value a_str;
    string_append(&a_str, a);
    return test_equals_function(as_string(&a_str), b, aText, bText, line, file);
}

void test_equals_function(Value* a, float b,
        const char* aText, const char* bText,
        int line, const char* file)
{
    return test_equals_function(to_float(a), b, aText, bText, line, file);
}

bool test_fail_on_static_error(Block* block)
{
    if (has_static_errors(block)) {
        std::cout << "Static error in " << get_current_test_name() << std::endl;
        Value str;
        print_static_errors_formatted(block, &str);
        std::cout << as_cstring(&str) << std::endl;
        declare_current_test_failed();
        return true;
    }
    return false;
}

bool test_fail_on_runtime_error(VM* vm)
{
    if (vm_has_error(vm)) {
        std::cout << "Runtime error in " << get_current_test_name() << std::endl;
        //circa_dump_stack_trace(&context);
        std::cout << std::endl;
        declare_current_test_failed();
        return true;
    }
    return false;
}

Value* temp_string(const char* str)
{
    if (gTempValue == NULL)
        gTempValue = new Value();
    set_string(gTempValue, str);
    return gTempValue;
}

World* test_world()
{
    return global_world();
}

void test_write_fake_file(const char* filename, int version, const char* contents)
{
    Value key;
    set_string(&key, filename);
    Value* entry = hashtable_insert(gFakeFileMap, &key);
    set_list(entry, 2);
    set_int(list_get(entry, 0), version);
    set_string(list_get(entry, 1), contents);
}

void before_each_test()
{
    if (gFakeFileMap == NULL)
        gFakeFileMap = new Value();
    set_mutable_hashtable(gFakeFileMap);

    caWorld* world = global_world();
    world_clear_file_sources(world);
    set_value(world_append_file_source(world), gFakeFileMap);
}

bool run_test(TestCase& testCase)
{
    gCurrentTestCase = testCase;

    before_each_test();
    testCase.execute();
    after_each_test();

    return !gCurrentTestCase.failed;
}

bool run_tests(std::string const& searchStr)
{
    int totalTestCount = 0;
    int successCount = 0;
    int failureCount = 0;
    std::vector<TestCase>::iterator it;
    for (it = gTestCases.begin(); it != gTestCases.end(); ++it) {
        if (it->name.find(searchStr) == std::string::npos)
            continue;
        totalTestCount++;
        std::cout << "Running " << it->name << std::endl;
        bool result = run_test(*it);
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
    int totalTestCount = 0;
    int successCount = 0;
    int failureCount = 0;
    std::vector<TestCase>::iterator it;
    for (it = gTestCases.begin(); it != gTestCases.end(); ++it) {
        totalTestCount++;
        bool result = run_test(*it);
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

void after_each_test()
{
    if (gTempValue != NULL)
        set_null(gTempValue);

    caWorld* world = global_world();
    world_clear_file_sources(world);
}

std::vector<std::string> list_all_test_names()
{
    std::vector<std::string> output;

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

namespace block_test { void register_tests(); }
namespace building_test { void register_tests(); }
namespace bytecode_test { void register_tests(); }
namespace code_iterator_test { void register_tests(); }
namespace compound_type_test { void register_tests(); }
namespace control_flow_test { void register_tests(); }
namespace fakefs_test { void register_tests(); }
namespace file_test { void register_tests(); }
namespace file_watch_test { void register_tests(); }
namespace function_test { void register_tests(); }
namespace hashtable_test { void register_tests(); }
namespace if_block_test { void register_tests(); }
namespace list_test { void register_tests(); }
namespace loop_test { void register_tests(); }
namespace migration_test { void register_tests(); }
namespace modules_test { void register_tests(); }
namespace names_test { void register_tests(); }
namespace native_patch_test { void register_tests(); }
namespace parser_test { void register_tests(); }
namespace path_expression_test { void register_tests(); }
namespace stack_test { void register_tests(); }
namespace string_test { void register_tests(); }
namespace symbol_test { void register_tests(); }
namespace tagged_value_test { void register_tests(); }
namespace tokenizer_test { void register_tests(); }
namespace type_test { void register_tests(); }

int main(int argc, char** argv)
{
    hashtable_test::register_tests();

    bytecode_test::register_tests();

#if 0
    block_test::register_tests();
    building_test::register_tests();
    code_iterator_test::register_tests();
    control_flow_test::register_tests();
    compound_type_test::register_tests();
    fakefs_test::register_tests();
    file_test::register_tests();
    file_watch_test::register_tests();
    function_test::register_tests();
    if_block_test::register_tests();
    list_test::register_tests();
    loop_test::register_tests();
    migration_test::register_tests();
    modules_test::register_tests();
    names_test::register_tests();
    native_patch_test::register_tests();
    parser_test::register_tests();
    path_expression_test::register_tests();
    stack_test::register_tests();
    symbol_test::register_tests();
    tagged_value_test::register_tests();
    string_test::register_tests();
    tokenizer_test::register_tests();
    type_test::register_tests();
#endif

    caWorld* world = circa_initialize();

    run_all_tests();

    circa_shutdown(world);
}
