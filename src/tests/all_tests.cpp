// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "all_tests.h"
#include "common.h"
#include "circa.h"

namespace circa {

void register_branch_tests();
void register_builtin_function_tests();
void register_container_tests();
void register_cpp_interface_tests();
void register_error_tests();
void register_execution_tests();
void register_feedback_tests();
void register_importing_tests();
void register_introspection_tests();
void register_list_tests();
void register_operations_tests();
void register_parser_tests();
void register_primitive_type_tests();
void register_subroutine_tests();
void register_tokenizer_tests();
void register_type_tests();

void register_all_tests()
{
    gTestCases.clear();

    register_branch_tests();
    register_builtin_function_tests();
    register_container_tests();
    register_cpp_interface_tests();
    register_error_tests();
    register_execution_tests();
    register_feedback_tests();
    register_importing_tests();
    register_introspection_tests();
    register_list_tests();
    register_operations_tests();
    register_parser_tests();
    register_primitive_type_tests();
    register_subroutine_tests();
    register_tokenizer_tests();
    register_type_tests();
}

void run_all_tests()
{
    register_all_tests();

    std::vector<TestCase>::iterator it;
    int totalTestCount = (int) gTestCases.size();
    int successCount = 0;
    int failureCount = 0;
    for (it = gTestCases.begin(); it != gTestCases.end(); ++it) {
        try {
            it->execute();
            successCount++;
        }
        catch (errors::CircaError &err) {
            std::cout << "Error white running test case " << it->name << std::endl;
            std::cout << err.message() << std::endl;
            failureCount++;
        }
    }

    std::string successes = successCount == 1 ? "success" : "sucesses";
    std::string failures = failureCount == 1 ? "failure" : "failures";

    std::cout << "Ran " << totalTestCount << " tests, " << successCount
        << " " << successes << " and " << failureCount << " "
        << failures << "." << std::endl;
}

std::vector<std::string> list_all_test_names()
{
    register_all_tests();

    std::vector<std::string> output;

    std::vector<TestCase>::iterator it;
    for (it = gTestCases.begin(); it != gTestCases.end(); ++it)
        output.push_back(it->name);

    return output;
}

}
