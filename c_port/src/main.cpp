
#include "common_headers.h"

#include "circa.h"
#include "tests/all_tests.h"

void run()
{
}

int main(int nargs, const char * args[])
{
    initialize();

    circa::run_all_tests();

    try {
        run();
    } catch (errors::CircaError &err)
    {
        std::cout << "Top level error:\n";
        std::cout << err.message() << std::endl;
    }
}

