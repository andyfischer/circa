
#include "common_headers.h"

#include "circa.h"
#include "tests/all_tests.h"

/*
void current_subroutine_name(Term* caller)
{
    Branch* branch = caller->owningBranch;
    if (branch == NULL) {
        as_string(caller) = "(null branch)";
        return;
    }

    Term* owningTerm* = branch->owningTerm;

    if (owningTerm* == NULL) {
        as_string(caller) = "(null owningTerm* on branch)";
        return;
    }

    if (!is_subroutine(owningTerm)) {
        as_string(caller) = string("(branch->owningTerm* has type: ")
            + as_type(owningTerm->type)->name + ")";
        return;
    }

    as_string(caller) = as_subroutine(owningTerm)->name;
}*/

void run()
{
    /*
    Branch* branch = new Branch();

    Term* sub = quick_exec_function(branch, "subroutine-create('test, list(), void)");
    Subroutine* subData = as_subroutine(sub);

    Term* csn = quick_create_function(branch, "current-subroutine-name",
            current_subroutine_name, TermList(), get_global("string"));

    quick_exec_function(as_subroutine(sub)->branch, "print(current-subroutine-name())");
    */
}

using namespace circa;

int main(int nargs, const char * args[])
{
    initialize();

    run_all_tests();

    try {
        run();
    } catch (errors::CircaError &err)
    {
        std::cout << "Top level error:\n";
        std::cout << err.message() << std::endl;
    }
}

