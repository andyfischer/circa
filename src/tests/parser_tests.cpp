// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "circa.h"

namespace circa {
namespace parser_tests {

void test_comment()
{
    Branch branch;
    parser::compile(&branch, parser::statement, "-- this is a comment");

    test_assert(branch[0]->function == COMMENT_FUNC);
    test_equals(branch[0]->stringProperty("comment"), "-- this is a comment");
    test_assert(branch.numTerms() == 1);

    parser::compile(&branch, parser::statement, "--");
    test_assert(branch.numTerms() == 2);
    test_assert(branch[1]->function == COMMENT_FUNC);
    test_equals(branch[1]->stringProperty("comment"), "--");
}

void test_blank_line()
{
    Branch branch;
    parser::compile(&branch, parser::statement, "");
    test_assert(branch.numTerms() == 1);
    test_assert(branch[0]->function == COMMENT_FUNC);
    test_equals(branch[0]->stringProperty("comment"), "\n");
}

void test_literal_integer()
{
    Branch branch;
    parser::compile(&branch, parser::statement, "1");
    test_assert(branch.numTerms() == 1);
    test_assert(is_value(branch[0]));
    test_assert(branch[0]->asInt() == 1);
}

void test_literal_float()
{
    Branch branch;
    parser::compile(&branch, parser::statement, "1.0");
    test_assert(branch.numTerms() == 1);
    test_assert(is_value(branch[0]));
    test_assert(branch[0]->asFloat() == 1.0);
}

void test_literal_string()
{
    Branch branch;
    parser::compile(&branch, parser::statement, "\"hello\"");
    test_assert(branch.numTerms() == 1);
    test_assert(is_value(branch[0]));
    test_assert(branch[0]->asString() == "hello");
}

void test_name_binding()
{
    Branch branch;
    parser::compile(&branch, parser::statement, "a = 1");
    test_assert(branch.numTerms() == 1);
    test_assert(is_value(branch[0]));
    test_assert(branch[0]->asInt() == 1);
    test_assert(branch[0] == branch["a"]);
    test_assert(branch[0]->name == "a");
}

void test_function_call()
{
    Branch branch;
    parser::compile(&branch, parser::statement, "add(1.0,2.0)");
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
    parser::compile(&branch, parser::statement, "a = 1.0");
    test_assert(branch.numTerms() == 1);

    Term* a = parser::compile(&branch, parser::statement, "a");

    test_assert(branch.numTerms() == 1);
    test_assert(a == branch[0]);

    parser::compile(&branch, parser::statement, "add(a,a)");
    test_assert(branch.numTerms() == 2);
    test_assert(branch[1]->input(0) == a);
    test_assert(branch[1]->input(1) == a);
}

void test_rebind()
{
    Branch branch;
    parser::compile(&branch, parser::statement, "a = 1.0");
    parser::compile(&branch, parser::statement, "add(@a,a)");

    test_assert(branch.numTerms() == 2);
    test_assert(branch["a"] == branch[1]);
}

void test_infix()
{
    Branch branch;
    parser::compile(&branch, parser::statement, "1.0 + 2.0");

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
    Term* typeTerm = parser::compile(&branch, parser::statement,
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
    Term* funcTerm = parser::compile(&branch, parser::statement,
            "def Myfunc(string what, string hey, int yo) -> bool\n"
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
    test_equals(funcbranch[5]->function->name, "greater_than");
    test_assert(funcbranch[5]->input(0) == funcbranch[2]);
    test_assert(funcbranch[5]->input(1) == funcbranch[4]);
    test_equals(funcbranch[5]->name, OUTPUT_PLACEHOLDER_NAME);
    test_assert(funcbranch.numTerms() == 6);
}

void test_stateful_value_decl()
{
    Branch branch;
    Term* a = parser::compile(&branch, parser::statement, "state a : int");

    test_assert(is_stateful(a));
    test_assert(a->name == "a");
    test_assert(a->type == INT_TYPE);
    test_assert(branch["a"] == a);
    test_assert(a->value != NULL);

    Term* b = parser::compile(&branch, parser::statement, "state b = 5.0");
    test_assert(b->name == "b");
    test_assert(is_stateful(b));
    test_assert(b->type == FLOAT_TYPE);
    test_assert(branch["b"] == b);
    test_assert(as_float(b) != 5.0); // shouldn't have this value yet

    Term* c = parser::compile(&branch, parser::statement, "state c :float = 7.5");
    test_assert(c->name == "c");
    test_assert(is_stateful(c));
    test_assert(c->type == FLOAT_TYPE);
    test_assert(branch["c"] == c);
    test_assert(as_float(c) != 7.5); // shouldn't have this value yet
}

void test_arrow_concatenation()
{
    Branch branch;
    Term* a = parser::compile(&branch, parser::statement, "1 -> to_string");

    test_assert(branch[0]->asInt() == 1);
    test_assert(branch[1] == a);
    test_equals(branch[1]->function->name, "to_string");
    test_assert(branch[1]->input(0) == branch[0]);
    test_assert(branch[1]->type == STRING_TYPE);
    test_assert(branch.numTerms() == 2);
}

void test_arrow_concatenation2()
{
    Branch branch;
    Term* a = parser::compile(&branch, parser::statement,
        "0.0 -> cos -> to_string");

    test_assert(branch[0]->asFloat() == 0.0);
    test_equals(branch[1]->function->name, "cos");
    test_assert(branch[1]->input(0) == branch[0]);
    test_equals(branch[2]->function->name, "to_string");
    test_assert(branch[2]->input(0) == branch[1]);
    test_assert(branch[2] == a);
    test_assert(branch.numTerms() == 3);
}

void test_dot_concatenation()
{
    Branch branch;

    parser::compile(&branch, parser::statement, "s = Set()");
    Term *s = parser::compile(&branch, parser::statement, "s.add(1)");

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
    Term* t = parser::compile(&branch, parser::function_call, "assert(false)");

    test_equals(t->stringProperty("syntaxHints:functionName"), "assert");

    t = parser::compile(&branch, parser::function_call, "concat('a', 'b')");
    test_equals(t->stringProperty("syntaxHints:functionName"), "concat");
    test_equals(get_input_syntax_hint(t, 0, "preWhitespace"), "");
    test_equals(get_input_syntax_hint(t, 0, "postWhitespace"), ",");
    test_equals(get_input_syntax_hint(t, 1, "preWhitespace"), " ");
    test_equals(get_input_syntax_hint(t, 1, "postWhitespace"), "");

    t = parser::compile(&branch, parser::statement, "x = true\n");
    test_equals(t->stringProperty("syntaxHints:postWhitespace"), "\n");
}

void test_implicit_copy_by_identifier()
{
    Branch branch;
    Term* a = branch.eval("a = 1");
    Term* b = branch.eval("b = a");

    test_assert(b->function == COPY_FUNC);
    test_assert(b->input(0) == a);
}

void test_rebinding_infix_operator()
{
    Branch branch;
    branch.eval("i = 1.0");
    Term* i = branch.eval("i += 1.0");

    test_assert(branch["i"] == i);
    test_assert(i->function == ADD_FUNC);
    test_assert(i->name == "i");
    test_assert(i->input(0)->name == "i");
}

void test_if_joining_on_bool()
{
    // The following code once had a bug where if_expr wouldn't work
    // if one of its inputs was missing value.
    Branch branch;
    Term* s = branch.eval("hey = true");

    test_assert(s->value != NULL);

    branch.eval("if false\nhey = false\nend");

    branch.eval();

    test_assert(branch["hey"]->asBool() == true);
}

void test_infix_whitespace()
{
    Branch branch;
    branch.eval("a = 1");
    branch.eval("b = 1");

    Term* term = parser::compile(&branch, parser::infix_expression, "  a + b");
    test_equals(term->stringProperty("syntaxHints:preWhitespace"), "  ");
    test_equals(get_input_syntax_hint(term, 0, "postWhitespace"), " ");
    test_equals(get_input_syntax_hint(term, 1, "preWhitespace"), " ");

    term = parser::compile(&branch, parser::infix_expression, "5+3");
    test_assert(term->stringProperty("syntaxHints:preWhitespace") == "");
    test_equals(get_input_syntax_hint(term, 0, "postWhitespace"), "");
    test_equals(get_input_syntax_hint(term, 1, "preWhitespace"), "");
    test_assert(term->stringProperty("syntaxHints:postWhitespace") == "");
}

void test_list_arguments()
{
    Branch branch;
    Term *t = branch.eval("add(1 2 3)");
    test_assert(as_int(t->input(0)) == 1);
    test_assert(as_int(t->input(1)) == 2);
    test_assert(as_int(t->input(2)) == 3);

    t = branch.eval("add(5\n 6 , 7;8)");
    test_assert(as_int(t->input(0)) == 5);
    test_assert(as_int(t->input(1)) == 6);
    test_assert(as_int(t->input(2)) == 7);
    test_assert(as_int(t->input(3)) == 8);
}

void test_function_decl_parse_error()
{
    Branch branch;
    Term* t = branch.eval("def !@#$");

    test_assert(t->function == UNRECOGNIZED_EXPRESSION_FUNC);
    test_assert(has_compile_error(t));
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
    REGISTER_TEST_CASE(parser_tests::test_rebinding_infix_operator);
    REGISTER_TEST_CASE(parser_tests::test_if_joining_on_bool);
    REGISTER_TEST_CASE(parser_tests::test_infix_whitespace);
    REGISTER_TEST_CASE(parser_tests::test_list_arguments);
    REGISTER_TEST_CASE(parser_tests::test_function_decl_parse_error);
}

} // namespace parser_tests
} // namespace circa
