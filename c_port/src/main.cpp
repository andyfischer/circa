
#include "common_headers.h"

#include "circa.h"
#include "errors.h"

void run()
{
    initialize();

    Term* five = create_constant(GetGlobal("int"));
    as_int(five) = 5;

    Term* five_as_string = create_term(GetGlobal("to-string"), TermList(five));
    five_as_string->execute();
    std::cout << "five = " << as_string(five_as_string) << std::endl;

    Term* my_struct_def = create_constant(GetGlobal("StructDefinition"));

    constant_string("hi");

    return;

    my_struct_def = apply_function(GetGlobal("add-field"),
            TermList(my_struct_def, constant_string("myInt"), GetGlobal("int")));
    my_struct_def = apply_function(GetGlobal("add-field"),
            TermList(my_struct_def, constant_string("myString"), GetGlobal("string")));

    Term* struct_instance = apply_function(my_struct_def, TermList());
}

int main(const char * args[])
{
    try {
        run();
    } catch (errors::CircaError &err)
    {
        std::cout << err.message() << std::endl;
    }
}
