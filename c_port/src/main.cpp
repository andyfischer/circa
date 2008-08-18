
#include "common_headers.h"

#include "circa.h"
#include "tests/all_tests.h"

using namespace circa;

void run()
{
    Branch* branch = new Branch();
    quick_exec_function(branch, "write-text-file(\"output.txt\", \"hello\")");
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
