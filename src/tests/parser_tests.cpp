// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "circa.h"

namespace circa {
namespace parser_tests {

void test_comment()
{
    Branch branch;
    parser::compile(branch, parser::statement, "-- this is a comment");

    test_assert(branch[0]->function == COMMENT_FUNC);
    test_equals(branch[0]->state->field(0)->asString(), " this is a comment");
    test_assert(branch.numTerms() == 1);

    parser::compile(branch, parser::statement, "--");
    test_assert(branch.numTerms() == 2);
    test_assert(branch[1]->function == COMMENT_FUNC);
    test_equals(branch[1]->state->field(0)->asString(), "");
}

void test_blank_line()
{
    Branch branch;
    parser::compile(branch, parser::statement, "");
    test_assert(branch.numTerms() == 1);
    test_assert(branch[0]->function == COMMENT_FUNC);
    test_equals(branch[0]->state->field(0)->asString(), "");
}

void test_literal_integer()
{
    Branch branch;
    parser::compile(branch, parser::statement, "1");
    test_assert(branch.numTerms() == 1);
    test_assert(is_value(branch[0]));
    test_assert(branch[0]->asInt() == 1);
}

void test_literal_float()
{
    Branch branch;
    parser::compile(branch, parser::statement, "1.0");
    test_assert(branch.numTerms() == 1);
    test_assert(is_value(branch[0]));
    test_assert(branch[0]->asFloat() == 1.0);
}

void test_literal_string()
{
    Branch branch;
    parser::compile(branch, parser::statement, "\"hello\"");
    test_assert(branch.numTerms() == 1);
    test_assert(is_value(branch[0]));
    test_assert(branch[0]->asString() == "hello");
}

void test_name_binding()
{
    Branch branch;
    parser::compile(branch, parser::statement, "a = 1");
    test_assert(branch.numTerms() == 1);
    test_assert(is_value(branch[0]));
    test_assert(branch[0]->asInt() == 1);
    test_assert(branch[0] == branch["a"]);
    test_assert(branch[0]->name == "a");
}

void test_function_call()
{
    Branch branch;
    parser::compile(branch, parser::statement, "add(1.0,2.0)");
    test_assert(branch.numTerms() == 3);
    test_assert(is_value(branch[0]));
    test_assert(branch[0]->asFloat() == 1.0);
    test_assert(is_value(branch[1]));
    test_assert(branch[1]->asFloat() == 2.0);

    test_assert(branch[2]->function == ADD_FUNC);
    test_assert(branch[2]->input(0) == branch[0]);
    test_assert(branch[2]->input(1) == branch[1]);
}

void test_identifier()
{
    Branch branch;
    parser::compile(branch, parser::statement, "a = 1.0");
    test_assert(branch.numTerms() == 1);

    Term* a = parser::compile(branch, parser::statement, "a");

    test_assert(branch.numTerms() == 1);
    test_assert(a == branch[0]);

    parser::compile(branch, parser::statement, "add(a,a)");
    test_assert(branch.numTerms() == 2);
    test_assert(branch[1]->input(0) == a);
    test_assert(branch[1]->input(1) == a);
}

void test_rebind()
{
    Branch branch;
    parser::compile(branch, parser::statement, "a = 1.0");
    parser::compile(branch, parser::statement, "add(@a,a)");

    test_assert(branch.numTerms() == 2);
    test_assert(branch["a"] == branch[1]);
}

void test_infix()
{
    Branch branch;
    parser::compile(branch, parser::statement, "1.0 + 2.0");

    test_assert(branch.numTerms() == 3);
    test_assert(branch[0]->asFloat() == 1.0);
    test_assert(branch[1]->asFloat() == 2.0);
    test_assert(branch[2]->function == ADD_FUNC);
    test_assert(branch[2]->input(0) == branch[0]);
    test_assert(branch[2]->input(1) == branch[1]);

    branch.clear();
}

void test_type_decl()
{
    Branch branch;
    Term* typeTerm = parser::compile(branch, parser::statement,
            "type Mytype {\nint a\nfloat b\n}");
    Type& type = as_type(typeTerm);

    test_equals(type.name, "Mytype");
    test_equals(typeTerm->name, "Mytype");

    test_assert(type.fields[0].type == INT_TYPE);
    test_equals(type.fields[0].name, "a");
    test_assert(type.fields[1].type == FLOAT_TYPE);
    test_equals(type.fields[1].name, "b");
}

void test_function_decl()
{
    Branch branch;
    Term* funcTerm = parser::compile(branch, parser::statement,
            "function Myfunc(string what, string hey, int yo) -> bool\n"
            "  whathey = concat(what,hey)\n"
            "  return yo > 3\n"
            "end");

    Function& func = as_function(funcTerm);

    test_equals(funcTerm->name, "Myfunc");
    test_equals(func.name, "Myfunc");

    test_assert(func.inputTypes[0] == STRING_TYPE);
    test_assert(func.getInputProperties(0).name == "what");
    test_assert(func.inputTypes[1] == STRING_TYPE);
    test_assert(func.getInputProperties(1).name == "hey");
    test_assert(func.inputTypes[2] == INT_TYPE);
    test_assert(func.getInputProperties(2).name == "yo");
    test_assert(func.outputType == BOOL_TYPE);

    Branch& funcbranch = func.subroutineBranch;

    test_equals(funcbranch[0]->name, "what");
    test_equals(funcbranch[1]->name, "hey");
    test_equals(funcbranch[2]->name, "yo");
    test_equals(funcbranch[3]->name, "whathey");
    test_equals(funcbranch[3]->function->name, "concat");
    test_assert(funcbranch[3]->input(0) == funcbranch[0]);
    test_assert(funcbranch[3]->input(1) == funcbranch[1]);
    test_assert(funcbranch[3]->input(1) == funcbranch[1]);
    test_assert(funcbranch[4]->asInt() == 3);
    test_equals(funcbranch[5]->function->name, "greater-than");
    test_assert(funcbranch[5]->input(0) == funcbranch[2]);
    test_assert(funcbranch[5]->input(1) == funcbranch[4]);
    test_equals(funcbranch[5]->name, "#return");
    test_assert(funcbranch.numTerms() == 6);
}

void test_stateful_value_decl()
{
    Branch branch;
    Term* a = parser::compile(branch, parser::statement, "state int a");

    test_assert(is_stateful(a));
    test_assert(a->type == INT_TYPE);
    test_assert(branch["a"] == a);
    test_assert(a->value != NULL);

    Term* b = parser::compile(branch, parser::statement, "state float b = 5.0");
    test_assert(is_stateful(b));
    test_assert(b->type == FLOAT_TYPE);
    test_assert(branch["b"] == b);
    test_assert(as_float(b) == 5.0);
}

void test_arrow_concatenation()
{
    Branch branch;
    Term* a = parser::compile(branch, parser::statement, "1 -> to-string");

    test_assert(branch[0]->asInt() == 1);
    test_assert(branch[1] == a);
    test_equals(branch[1]->function->name, "to-string");
    test_assert(branch[1]->input(0) == branch[0]);
    test_assert(branch[1]->type == STRING_TYPE);
    test_assert(branch.numTerms() == 2);
}

void test_arrow_concatenation2()
{
    Branch branch;
    Term* a = parser::compile(branch, parser::statement,
        "\"hello\" -> tokenize -> to-string");

    test_assert(branch[0]->asString() == "hello");
    test_equals(branch[1]->function->name, "tokenize");
    test_assert(branch[1]->input(0) == branch[0]);
    test_equals(branch[2]->function->name, "to-string");
    test_assert(branch[2]->input(0) == branch[1]);
    test_assert(branch[2] == a);
    test_assert(branch.numTerms() == 3);
}

void test_dot_concatenation()
{
    Branch branch;

    parser::compile(branch, parser::statement, "s = Set()");
    Term *s = parser::compile(branch, parser::statement, "s.add(1)");

    test_assert(branch.numTerms() == 3);
    test_assert(is_value(branch[0]));
    test_assert(is_value(branch[1]));
    test_assert(branch[1]->asInt() == 1);
    test_equals(as_function(branch[2]->function).name, "add");
    test_assert(branch["s"] == s);
}

void test_syntax_hints()
{
    Branch branch;
    Term* t = parser::compile(branch, parser::function_call, "assert(false)");

    test_assert(t->syntaxHints.functionName == "assert");
    test_assert(t->syntaxHints.getInputSyntax(0).style
            == TermSyntaxHints::InputSyntax::BY_NAME);
    test_assert(t->syntaxHints.getInputSyntax(0).name == "false");
}

void test_implicit_copy_by_identifier()
{
    Branch branch;
    Term* a = branch.eval("a = 1");
    Term* b = branch.eval("b = a");

    test_assert(b->function == COPY_FUNC);
    test_assert(b->input(0) == a);
}

void register_tests()
{
    REGISTER_TEST_CASE(parser_tests::test_comment);
    REGISTER_TEST_CASE(parser_tests::test_blank_line);
    REGISTER_TEST_CASE(parser_tests::test_literal_integer);
    REGISTER_TEST_CASE(parser_tests::test_literal_float);
    REGISTER_TEST_CASE(parser_tests::test_literal_string);
    REGISTER_TEST_CASE(parser_tests::test_name_binding);
    REGISTER_TEST_CASE(parser_tests::test_function_call);
    REGISTER_TEST_CASE(parser_tests::test_identifier);
    REGISTER_TEST_CASE(parser_tests::test_rebind);
    REGISTER_TEST_CASE(parser_tests::test_infix);
    REGISTER_TEST_CASE(parser_tests::test_type_decl);
    REGISTER_TEST_CASE(parser_tests::test_function_decl);
    REGISTER_TEST_CASE(parser_tests::test_stateful_value_decl);
    REGISTER_TEST_CASE(parser_tests::test_arrow_concatenation);
    REGISTER_TEST_CASE(parser_tests::test_arrow_concatenation2);
    REGISTER_TEST_CASE(parser_tests::test_dot_concatenation);
    REGISTER_TEST_CASE(parser_tests::test_syntax_hints);
    REGISTER_TEST_CASE(parser_tests::test_implicit_copy_by_identifier);
}

} // namespace parser_tests
} // namespace circa
