
#include "common_headers.h"

#include "circa.h"
#include "tests/all_tests.h"

void run()
{
    initialize();

    Branch* branch = new Branch();

    Term* five = create_constant(branch, get_global("int"));
    as_int(five) = 5;

    Term* five_as_string = create_term(branch, get_global("to-string"), TermList(five));
    execute(five_as_string);
    std::cout << "five = " << as_string(five_as_string) << std::endl;

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

    Term* hopefully_two = quick_eval_function(branch,
            "hopefully-two = get-field(my_instance, \"myInt\")");

    //Term* hopefully_two = apply_function(branch, get_global("get-field"),
            //TermList(my_instance, constant_string(branch, "myInt")));
    execute(hopefully_two);
    //branch->bindName(hopefully_two, "hopefully-two");

    Term* hopefully_two_as_string = apply_function(branch, get_global("to-string"),
            TermList(hopefully_two));
    execute(hopefully_two_as_string);

    branch->bindName(hopefully_two_as_string, "a");

    Term* print_hopefully_two = quick_eval_function(branch, "print(a)");
    execute(print_hopefully_two);
    
    Term* br = apply_function(branch, get_global("Branch"), TermList());
    execute(br);
}

int main(const char * args[])
{
    // todo: figure out a good way to process args
    run_all_tests();

    try {
        run();
    } catch (errors::CircaError &err)
    {
        std::cout << "Top level error:\n";
        std::cout << err.message() << std::endl;
    }
}
