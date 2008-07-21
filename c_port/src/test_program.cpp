
#include "common_headers.h"

#include "circa.h"

int main(const char * args[])
{
    initialize();

    Term* five = create_constant(GetGlobal("int"));
    as_int(five) = 5;

    Term* five_as_string = create_term(GetGlobal("to-string"), TermList(five));
    five_as_string->execute();
    std::cout << "five = " << as_string(five_as_string) << std::endl;
}
