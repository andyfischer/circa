
#include "common_headers.h"

#include "bootstrapping.h"
#include "branch.h"
#include "builtins.h"
#include "common.h"
#include "errors.h"
#include "operations.h"
#include "subroutine.h"

namespace circa {
namespace struct_test {

void test_simple()
{
    Branch *branch = new Branch();
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
            "hopefully-two = get-field(my_instance, 'myInt)");

    test_assert(as_int(hopefully_two) == 2);
}

void test_simple2()
{
    Branch branch;
    quick_exec_function(&branch,
        "my-struct-def = define-struct('my-struct-def, list(int, string, Subroutine))");
    Term* myinst = quick_exec_function(&branch, "my-inst = my-struct-def()");
    myinst->toString();
}

void function_body_whatever(Term*) {}

void test_as_function_output()
{
    Branch *branch = new Branch();

    Term* return_type = quick_exec_function(branch, "define-struct('rt, list(string,int))");
    quick_create_function(branch, "my-func", function_body_whatever,
            TermList(INT_TYPE), return_type);

    Term* func_inst = quick_exec_function(branch, "x = my-func(5)");

    test_assert(func_inst->type == return_type);
    test_assert(quick_exec_function(branch, "get-index(x, 0)")->type == STRING_TYPE);
    test_assert(quick_exec_function(branch, "get-index(x, 1)")->type == INT_TYPE);
}

void all_tests()
{
    test_simple();
    test_simple2();
    test_as_function_output();
}

} // namespace struct_test
} // namespace circa
