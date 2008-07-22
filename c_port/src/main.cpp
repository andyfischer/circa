
#include "common_headers.h"

#include "circa.h"
#include "errors.h"

void run()
{
    initialize();

    Term* five = create_constant(GetGlobal("int"));
    as_int(five) = 5;

    Term* five_as_string = create_term(GetGlobal("to-string"), TermList(five));
    execute(five_as_string);
    std::cout << "five = " << as_string(five_as_string) << std::endl;

    Term* my_struct_def = create_constant(GetGlobal("StructDefinition"));
    my_struct_def = apply_function(GetGlobal("struct-definition-set-name"),
            TermList(my_struct_def, constant_string("my-struct")));
    execute(my_struct_def);

    my_struct_def = apply_function(GetGlobal("add-field"),
            TermList(my_struct_def, constant_string("myInt"), GetGlobal("int")));
    execute(my_struct_def);
    my_struct_def = apply_function(GetGlobal("add-field"),
            TermList(my_struct_def, constant_string("myString"), GetGlobal("string")));
    execute(my_struct_def);

    Term* my_instance = apply_function(my_struct_def, TermList());
    execute(my_instance);

    my_instance = apply_function(GetGlobal("set-field"),
            TermList(my_instance, constant_string("myInt"), constant_int(2)));
    execute(my_instance);

    Term* hopefully_two = apply_function(GetGlobal("get-field"),
            TermList(my_instance, constant_string("myInt")));
    execute(hopefully_two);
    
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
