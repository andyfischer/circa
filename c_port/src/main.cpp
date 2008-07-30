
#include "common_headers.h"

#include "circa.h"
#include "tests/all_tests.h"

void run()
{
    Branch* branch = new Branch();


}

int main(const char * args[])
{
    initialize();
	
    // todo: figure out a good way to process args
    circa::run_all_tests();

    try {
        run();
    } catch (errors::CircaError &err)
    {
        std::cout << "Top level error:\n";
        std::cout << err.message() << std::endl;
    }
}

