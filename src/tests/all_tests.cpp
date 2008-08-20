
#include "common_headers.h"

#include "all_tests.h"
#include "errors.h"

namespace circa {

namespace tokenizer_test { void all_tests(); }
namespace subroutine_test { void all_tests(); }
namespace builtin_function_test { void all_tests(); }
namespace struct_test { void all_tests(); }
namespace primitive_type_test { void all_tests(); }
namespace list_test { void all_tests(); }
namespace branch_test { void all_tests(); }

void run_all_tests()
{
    typedef void (*RunTestsFunc)();
    std::vector<RunTestsFunc> tests;

    tests.push_back(tokenizer_test::all_tests);
    tests.push_back(subroutine_test::all_tests);
    tests.push_back(builtin_function_test::all_tests);
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
