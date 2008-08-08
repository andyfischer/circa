
#include "common_headers.h"

#include "all_tests.h"
#include "errors.h"

namespace circa {

void tokenize_test();
void subroutine_test();
void builtin_functions_test();

namespace struct_test { void struct_test(); }
namespace primitive_type_test { void primitive_type_test(); }
namespace list_test { void list_test(); }
namespace branch_test { void all_tests(); }

void run_all_tests()
{
    try {
        tokenize_test();
        subroutine_test();
        builtin_functions_test();
        struct_test::struct_test();
        primitive_type_test::primitive_type_test();
        list_test::list_test();
        branch_test::all_tests();
    } 
    catch (errors::CircaError &err) {
        std::cout << "Error during tests:\n";
        std::cout << err.message() << std::endl;
    }
}

}
