
#include "common_headers.h"

#include "circa.h"
#include "errors.h"

void run()
{
    initialize();

    Term* five = create_constant(get_global("int"));
    as_int(five) = 5;

    Term* five_as_string = create_term(get_global("to-string"), TermList(five));
    execute(five_as_string);
    std::cout << "five = " << as_string(five_as_string) << std::endl;

    Term* my_struct_def = create_constant(get_global("StructDefinition"));
    my_struct_def = apply_function(get_global("struct-definition-set-name"),
            TermList(my_struct_def, constant_string("my-struct")));
    execute(my_struct_def);

    my_struct_def = apply_function(get_global("add-field"),
            TermList(my_struct_def, constant_string("myInt"), get_global("int")));
    execute(my_struct_def);
    my_struct_def = apply_function(get_global("add-field"),
            TermList(my_struct_def, constant_string("myString"), get_global("string")));
    execute(my_struct_def);

    Term* my_instance = apply_function(my_struct_def, TermList());
    execute(my_instance);

    my_instance = apply_function(get_global("set-field"),
            TermList(my_instance, constant_string("myInt"), constant_int(2)));
    execute(my_instance);

    Term* hopefully_two = apply_function(get_global("get-field"),
            TermList(my_instance, constant_string("myInt")));
    execute(hopefully_two);
    
    std::cout << "hopefully two = " << hopefully_two->toString() << std::endl;

    Term* br = apply_function(get_global("Branch"), TermList());
    execute(br);


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
