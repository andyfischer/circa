
#include "common_headers.h"

#include "circa.h"
#include "tests/all_tests.h"

void run()
{
    Branch* branch = new Branch();

    Term* my_struct_def = create_constant(branch, get_global("StructDefinition"));
    my_struct_def = apply_function(branch, get_global("struct-definition-set-name"),
            TermList(my_struct_def, constant_string(branch, "my-struct")));
    execute(my_struct_def);

    my_struct_def = apply_function(branch, get_global("add-field"),
            TermList(my_struct_def, constant_string(branch, "myInt"), get_global("int")));
    execute(my_struct_def);
    my_struct_def = apply_function(branch, get_global("add-field"),
            TermList(my_struct_def, constant_string(branch, "myString"), get_global("string")));
    execute(my_struct_def);

    Term* my_instance = apply_function(branch, my_struct_def, TermList());
    execute(my_instance);

    my_instance = apply_function(branch, get_global("set-field"),
            TermList(my_instance, constant_string(branch, "myInt"),
                constant_int(branch, 2)));
    execute(my_instance);
    branch->bindName(my_instance, "my_instance");

    Term* hopefully_two = quick_exec_function(branch,
            "hopefully-two = get-field(my_instance, \"myInt\")");

    quick_exec_function(branch, "print(\"Hopefully two: \")");
    quick_exec_function(branch, "print(to-string(hopefully-two))");
    //quick_exec_function(branch, "print(ht-as-string)");
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

