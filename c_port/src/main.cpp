
#include "common_headers.h"

#include "circa.h"
#include "tests/all_tests.h"

void run()
{
    Branch* branch = new Branch();
    Term* any_t = get_global("any");
    Term* void_t = get_global("void");

    Term* print_term = quick_create_subroutine(branch, "print-term", TermList(any_t), void_t);

    Term* input_names = constant_list(branch, TermList(constant_string(branch, "Term")));
    print_term = exec_function(branch, get_global("subroutine-name-inputs"),
            TermList(print_term, input_names));
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

