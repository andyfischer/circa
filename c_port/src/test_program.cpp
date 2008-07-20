
#include "common_headers.h"

#include "circa.h"

int main(const char * args[])
{
    initialize();

    Term* five = KERNEL->createConstant(GetGlobal("int"), NULL);
    as_int(five) = 5;

    Term* five_as_string = KERNEL->createTerm(GetGlobal("to-string"), TermList(five), NULL);
    five_as_string->execute();
    std::cout << "five = " << as_string(five_as_string) << std::endl;
}
