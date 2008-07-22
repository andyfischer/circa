
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
    my_struct_def = apply_function(GetGlobal("struct-definition-set-name"),
            TermList(my_struct_def, constant_string("my-struct")));
    my_struct_def->execute();

    my_struct_def = apply_function(GetGlobal("add-field"),
            TermList(my_struct_def, constant_string("myInt"), GetGlobal("int")));
    my_struct_def->execute();
    my_struct_def = apply_function(GetGlobal("add-field"),
            TermList(my_struct_def, constant_string("myString"), GetGlobal("string")));
    my_struct_def->execute();

    Term* my_instance = apply_function(my_struct_def, TermList());
    my_instance->execute();

    my_instance = apply_function(GetGlobal("set-field"),
            TermList(my_instance, constant_string("myInt"), constant_int(2)));
    my_instance->execute();

    Term* hopefully_two = apply_function(GetGlobal("get-field"),
            TermList(my_instance, constant_string("myInt")));
    hopefully_two->execute();
    
    std::cout << "hopefully two = " << hopefully_two->toString() << std::endl;
}

int main(const char * args[])
{
    try {
        run();
    } catch (errors::CircaError &err)
    {
        std::cout << "Top level error:\n";
        std::cout << err.message() << std::endl;
    }
}
