
#include "common_headers.h"

#include "all_tests.h"
#include "errors.h"

void tokenize_test();
void subroutine_test();
void builtin_functions_test();

void run_all_tests()
{
    try {
        tokenize_test();
        subroutine_test();
        builtin_functions_test();
    } 
    catch (errors::CircaError &err) {
        std::cout << "Error during tests:\n";
        std::cout << err.message() << std::endl;
    }
}
