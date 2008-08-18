
#include "common_headers.h"

#include "circa.h"
#include "tests/all_tests.h"

using namespace circa;

void run()
{
    Branch* branch = new Branch();
    quick_exec_function(branch, "s = read-text-file('test)");
    quick_exec_function(branch, "s2 = concat(s, \" there\")");
    quick_exec_function(branch, "print(s2)");
    quick_exec_function(branch, "write-text-file('test, s2)");
}

int main(int nargs, const char * args[])
{
    initialize();

    bool runTests = false;

    if (runTests) {
        run_all_tests();
        std::cout << "All tests finished" << std::endl;
    }

    try {
        run();
    } catch (errors::CircaError &err)
    {
        std::cout << "Top level error:\n";
        std::cout << err.message() << std::endl;
    }
}
