
#include "common_headers.h"
#include "builtins.h"
#include "bootstrapping.h"
#include "operations.h"
#include "parser.h"
#include "common.h"
#include "structs.h"

namespace circa {
namespace struct_tests {

void test_create_instance()
{
    StructDefinition *def = new StructDefinition();

    def->addField("int1", INT_TYPE);
    def->addField("str1", STRING_TYPE);

    test_assert(def->numFields() == 2);
    test_assert(def->getType(0) == INT_TYPE);
    test_assert(def->getType(1) == STRING_TYPE);

    StructInstance *inst = new StructInstance(*def);

    test_assert(inst->getField(0)->type == INT_TYPE);
    test_assert(inst->getField(1)->type == STRING_TYPE);

    delete inst;
    delete def;
}

void test_misc()
{
    Branch branch;
    Term* my_struct_def = create_constant(&branch, get_global("StructDefinition"));
    my_struct_def = apply_function(&branch, get_global("struct-definition-set-name"),
            TermList(my_struct_def, constant_string(&branch, "my-struct")));
    my_struct_def->eval();

    my_struct_def = apply_function(&branch, get_global("add-field"),
            TermList(my_struct_def, constant_string(&branch, "myFloat"), FLOAT_TYPE));
    my_struct_def->eval();
    my_struct_def = apply_function(&branch, get_global("add-field"),
            TermList(my_struct_def, constant_string(&branch, "myString"), STRING_TYPE));
    my_struct_def->eval();

    Term* my_instance = apply_function(&branch, my_struct_def, TermList());
    my_instance->eval();

    my_instance = apply_function(&branch, get_global("set-field"),
            TermList(my_instance, constant_string(&branch, "myFloat"),
                constant_float(&branch, 2)));
    my_instance->eval();
    branch.bindName(my_instance, "my_instance");

    Term* hopefully_two = parser::eval_statement(&branch,
            "hopefully-two = get-field(my_instance, 'myFloat)");

    test_assert(as_float(hopefully_two) == 2);
}

void test_misc2()
{
    Branch branch;
    parser::eval_statement(&branch,
        "my-struct-def = define-struct('my-struct-def, list(float, string, Subroutine))");
    Term* myinst = parser::eval_statement(&branch, "my-inst = my-struct-def()");
    myinst->toString();
}

void function_body_whatever(Term*) {}

void test_as_function_output()
{
    Branch *branch = new Branch();

    Term* return_type = parser::eval_statement(branch, "define-struct('rt, list(string,float))");
    quick_create_function(branch, "my-func", function_body_whatever,
            TermList(FLOAT_TYPE), return_type);

    Term* func_inst = parser::eval_statement(branch, "x = my-func(5)");

    test_assert(func_inst->type == return_type);
    test_assert(parser::eval_statement(branch, "get-index(x, 0)")->type == STRING_TYPE);
    test_assert(parser::eval_statement(branch, "get-index(x, 1)")->type == FLOAT_TYPE);
}

void test_rename_field()
{
    Branch *branch = new Branch();
    parser::eval_statement(branch, "s = define-struct('s, list(float,List))");
    parser::eval_statement(branch, "s = struct-definition-rename-field(s, 0, 'FirstField)");
    parser::eval_statement(branch, "s = struct-definition-rename-field(s, 1, 'SecondField)");
    parser::eval_statement(branch, "inst = s()");
    Term* firstField = parser::eval_statement(branch, "get-field(inst, 'FirstField)");

    test_assert(firstField != NULL);
    test_assert(firstField->type == FLOAT_TYPE);
}

} // namespace struct_tests

void register_struct_tests()
{
    REGISTER_TEST_CASE(struct_tests::test_create_instance);
    REGISTER_TEST_CASE(struct_tests::test_misc);
    REGISTER_TEST_CASE(struct_tests::test_misc2);
    REGISTER_TEST_CASE(struct_tests::test_as_function_output);
    REGISTER_TEST_CASE(struct_tests::test_rename_field);
}

} // namespace circa
