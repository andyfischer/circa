
#include "common_headers.h"

#include "all_tests.h"
#include "errors.h"

namespace circa {

void tokenize_test();
void subroutine_test();
void builtin_functions_test();

namespace struct_test { void struct_test(); }

void run_all_tests()
{
    try {
        tokenize_test();
        subroutine_test();
        builtin_functions_test();
        struct_test::struct_test();
    } 
    catch (errors::CircaError &err) {
        std::cout << "Error during tests:\n";
        std::cout << err.message() << std::endl;
    }
}

}
