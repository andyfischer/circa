
#include "common_headers.h"

#include "all_tests.h"
#include "errors.h"

namespace circa {

void tokenize_test();
void subroutine_test();
void builtin_functions_test();

namespace struct_test { void all_tests(); }
namespace primitive_type_test { void all_tests(); }
namespace list_test { void all_tests(); }
namespace branch_test { void all_tests(); }

void run_all_tests()
{
    typedef void (*RunTestsFunc)();
    std::vector<RunTestsFunc> tests;

    tests.push_back(tokenize_test);
    tests.push_back(subroutine_test);
    tests.push_back(builtin_functions_test);
    tests.push_back(struct_test::all_tests);
    tests.push_back(primitive_type_test::all_tests);
    tests.push_back(list_test::all_tests);
    tests.push_back(branch_test::all_tests);

    std::vector<RunTestsFunc>::iterator it;
    for (it = tests.begin(); it != tests.end(); ++it) {
        try {
            (*it)();
        }
        catch (errors::CircaError &err) {
            std::cout << "Error during tests:\n";
            std::cout << err.message() << std::endl;
        }
    }
}

}
