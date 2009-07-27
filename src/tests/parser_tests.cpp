// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "circa.h"

namespace circa {
namespace parser_tests {

void test_comment()
{
    Branch branch;
    parser::compile(&branch, parser::statement, "-- this is a comment");

    test_assert(branch[0]->function == COMMENT_FUNC);
    test_equals(branch[0]->stringProp("comment"), "-- this is a comment");
    test_assert(branch.length() == 1);

    parser::compile(&branch, parser::statement, "--");
    test_assert(branch.length() == 2);
    test_assert(branch[1]->function == COMMENT_FUNC);
    test_equals(branch[1]->stringProp("comment"), "--");
}

void test_blank_line()
{
    Branch branch;
    parser::compile(&branch, parser::statement, "\n");
    test_assert(branch.length() == 1);
    test_assert(branch[0]->function == COMMENT_FUNC);
    test_equals(branch[0]->stringProp("comment"), "");
}

void test_literal_integer()
{
    Branch branch;
    parser::compile(&branch, parser::statement, "1");
    test_assert(branch.length() == 1);
    test_assert(is_value(branch[0]));
    test_assert(branch[0]->asInt() == 1);
}

void test_literal_float()
{
    Branch branch;
    parser::compile(&branch, parser::statement, "1.0");
    test_assert(branch.length() == 1);
    test_assert(is_value(branch[0]));
    test_assert(branch[0]->asFloat() == 1.0);

    Term* b = parser::compile(&branch, parser::statement_list, "5.200");
    test_assert(b->type == FLOAT_TYPE);
    test_assert(to_string(b) == "5.200");
}

void test_literal_string()
{
    Branch branch;
    parser::compile(&branch, parser::statement, "\"hello\"");
    test_assert(branch.length() == 1);
    test_assert(is_value(branch[0]));
    test_assert(branch[0]->asString() == "hello");
}

void test_name_binding()
{
    Branch branch;
    parser::compile(&branch, parser::statement, "a = 1");
    test_assert(branch.length() == 1);
    test_assert(is_value(branch[0]));
    test_assert(branch[0]->asInt() == 1);
    test_assert(branch[0] == branch["a"]);
    test_assert(branch[0]->name == "a");
}

void test_function_call()
{
    Branch branch;
    parser::compile(&branch, parser::statement, "add_f(1.0,2.0)");
    test_assert(branch.length() == 3);
    test_assert(is_value(branch[0]));
    test_assert(branch[0]->asFloat() == 1.0);
    test_assert(is_value(branch[1]));
    test_assert(branch[1]->asFloat() == 2.0);

    test_assert(branch[2]->function->name == "add_f");
    test_assert(branch[2]->input(0) == branch[0]);
    test_assert(branch[2]->input(1) == branch[1]);
}

void test_identifier()
{
    Branch branch;
    Term* a = parser::compile(&branch, parser::statement, "a = 1.0");
    test_assert(branch.length() == 1);

    test_assert(branch.length() == 1);
    test_assert(a == branch[0]);

    parser::compile(&branch, parser::statement, "add(a,a)");
    test_assert(branch.length() == 2);
    test_assert(branch[1]->input(0) == a);
    test_assert(branch[1]->input(1) == a);
}

void test_rebind()
{
    Branch branch;
    parser::compile(&branch, parser::statement, "a = 1.0");
    parser::compile(&branch, parser::statement, "add(@a,a)");

    test_assert(branch.length() == 2);
    test_assert(branch["a"] == branch[1]);
}

void test_infix()
{
    Branch branch;
    parser::compile(&branch, parser::statement, "1.0 + 2.0");

    test_assert(branch.length() == 3);
    test_assert(branch[0]->asFloat() == 1.0);
    test_assert(branch[1]->asFloat() == 2.0);
    test_assert(branch[2]->function->name == "add_f");
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

    test_assert(type.prototype[0]->type == INT_TYPE);
    test_equals(type.prototype[0]->name, "a");
    test_assert(type.prototype[1]->type == FLOAT_TYPE);
    test_equals(type.prototype[1]->name, "b");
}

void test_function_decl()
{
    Branch branch;
    Term* func = parser::compile(&branch, parser::statement,
            "def Myfunc(string what, string hey, int yo) : bool\n"
            "  whathey = concat(what,hey)\n"
            "  return yo > 3\n"
            "end");

    test_equals(func->name, "Myfunc");
    test_equals(function_t::get_name(func), "Myfunc");

    test_assert(function_t::get_input_type(func, 0) == STRING_TYPE);
    test_assert(function_t::get_input_name(func, 0) == "what");
    test_assert(function_t::get_input_type(func, 1) == STRING_TYPE);
    test_assert(function_t::get_input_name(func, 1) == "hey");
    test_assert(function_t::get_input_type(func, 2) == INT_TYPE);
    test_assert(function_t::get_input_name(func, 2) == "yo");
    test_assert(function_t::get_output_type(func) == BOOL_TYPE);

    Branch& funcbranch = as_branch(func);

    // index 0 has the function definition
    test_equals(funcbranch[1]->name, "what");
    test_equals(funcbranch[2]->name, "hey");
    test_equals(funcbranch[3]->name, "yo");
    test_equals(funcbranch[4]->name, "whathey");
    test_equals(funcbranch[4]->function->name, "concat");
    test_assert(funcbranch[4]->input(0) == funcbranch[1]);
    test_assert(funcbranch[4]->input(1) == funcbranch[2]);
    test_assert(funcbranch[4]->input(1) == funcbranch[2]);
    test_assert(funcbranch[5]->asInt() == 3);
    test_equals(funcbranch[6]->function->name, "greater_than");
    test_assert(funcbranch[6]->input(0) == funcbranch[3]);
    test_assert(funcbranch[6]->input(1) == funcbranch[5]);
    test_equals(funcbranch[6]->name, OUTPUT_PLACEHOLDER_NAME);
    test_assert(funcbranch.length() == 7);

    // This string once caused an error
    Term* a = branch.eval("def f()\n  end");
    test_assert(a);
}

void test_stateful_value_decl()
{
    Branch branch;
    Term* a = parser::compile(&branch, parser::statement, "state a : int");

    test_assert(is_stateful(a));
    test_assert(a->name == "a");
    test_assert(a->type == INT_TYPE);
    test_assert(branch["a"] == a);
    test_assert(is_value_alloced(a));

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
    test_assert(branch.length() == 2);
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
    test_assert(branch.length() == 3);
}

void test_dot_concatenation()
{
    Branch branch;

    branch.eval("s = Set()");

    // This function should rebind 's'
    Term *s = branch.eval("s.add(1)");

    test_assert(branch.length() == 3);
    test_assert(is_value(branch[0]));
    test_assert(is_value(branch[1]));
    test_assert(branch[1]->asInt() == 1);
    test_equals(function_t::get_name(branch[2]->function), "add");
    test_assert(branch["s"] == s);
}

void test_syntax_hints()
{
    Branch branch;

    Term* t = parser::compile(&branch, parser::statement, "concat('a', 'b')");
    test_equals(get_input_syntax_hint(t, 0, "preWhitespace"), "");
    test_equals(get_input_syntax_hint(t, 0, "postWhitespace"), ",");
    test_equals(get_input_syntax_hint(t, 1, "preWhitespace"), " ");
    test_equals(get_input_syntax_hint(t, 1, "postWhitespace"), "");
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
    test_assert(i->function->name == "add_f");
    test_assert(i->name == "i");
    test_assert(i->input(0)->name == "i");
}

void test_infix_whitespace()
{
    Branch branch;
    branch.eval("a = 1");
    branch.eval("b = 1");

    Term* term = parser::compile(&branch, parser::infix_expression, "  a + b");
    test_equals(term->stringProp("syntaxHints:preWhitespace"), "  ");
    test_equals(get_input_syntax_hint(term, 0, "postWhitespace"), " ");
    test_equals(get_input_syntax_hint(term, 1, "preWhitespace"), " ");

    term = parser::compile(&branch, parser::infix_expression, "5+3");
    test_assert(term->stringProp("syntaxHints:preWhitespace") == "");
    test_equals(get_input_syntax_hint(term, 0, "postWhitespace"), "");
    test_equals(get_input_syntax_hint(term, 1, "preWhitespace"), "");
    test_assert(term->stringProp("syntaxHints:postWhitespace") == "");
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
    test_assert(has_static_error(t));
}

void test_semicolon_as_line_ending()
{
    Branch branch;
    branch.compile("1;2;3");
    test_assert(!has_static_errors(branch));
    test_assert(branch.length() == 3);
    test_assert(is_value(branch[0]));
    test_assert(is_value(branch[1]));
    test_assert(is_value(branch[2]));
    test_assert(branch[0]->asInt() == 1);
    test_assert(branch[1]->asInt() == 2);
    test_assert(branch[2]->asInt() == 3);

    branch.clear();
    branch.compile("a = 1+2 ; b = mult(3,4) ; b -> print");
    test_assert(!has_static_errors(branch));
    test_assert(branch.length() == 7);
    test_assert(branch["a"]->function->name == "add_i");
    test_assert(branch["b"]->function->name == "mult_i");

    branch.clear();
    branch.compile("cond = true; if cond; a = 1; else; a = 2; end");

    test_assert(!has_static_errors(branch));
    evaluate_branch(branch);
    test_assert(branch.contains("a"));
    test_assert(branch["a"]->asInt() == 1);
    test_assert(branch.contains("cond"));
    branch["cond"]->asBool() = false;
    evaluate_branch(branch);
    test_assert(branch["a"]->asInt() == 2);
}

void test_unary_minus()
{
    Branch branch;
    Term* a = branch.eval("a = 1");
    Term* b = branch.eval("b = -a");

    test_assert(b->function->name == "neg_i");
    test_assert(b->input(0) == a);
    test_equals(b->toFloat(), -1.0);

    // Sometimes, literals with a - sign are supposed to turn that into a minus operation
    //TODO
    //Term* c = branch.eval("2-1");
    //test_assert(c->function->name == "sub_i");
    //test_assert(c->asInt() == 1);

    // Sometimes the - sign is part of the number
    Term* d = branch.eval("-3");
    test_assert(is_value(d));
    test_assert(d->asInt() == -3);
}

void test_array_index_access()
{
    Branch branch;
    branch.eval("a = [1 2 3]");
    Term* b = branch.eval("a[0]");

    test_assert(b);
    test_assert(b->function == GET_INDEX_FUNC);
    test_assert(b->asInt() == 1);
}

void test_float_division()
{
    Branch branch;
    Term* a = branch.eval("5 / 3");

    test_assert(a->type == FLOAT_TYPE);
    test_assert(a->function->name == "div");
    test_equals(a->toFloat(), 5.0f/3.0f);
}

void test_integer_division()
{
    Branch branch;
    Term* a = branch.eval("5 // 3");

    test_assert(a->type == INT_TYPE);
    test_assert(a->function->name == "div_i");
    test_assert(a->asInt() == 1);
}

void test_literal_list()
{
    Branch branch;
    Term* a = branch.eval("a = 1");
    Term* l = branch.eval("l = [1+2, a, sqrt(sqr(a))]");

    // Make sure that the extra values are created outside of the list
    // and in the proper order.
    test_assert(branch[0] == a);
    test_assert(branch[1]->asInt() == 1);
    test_assert(branch[2]->asInt() == 2);
    test_assert(branch[3]->function->name == "sqr");
    test_assert(branch[4] == l);

    // Look at the list contents
    test_assert(as_branch(l)[0]->function->name == "add_i");
    test_assert(as_branch(l)[1]->function->name == "copy");
    test_assert(as_branch(l)[2]->function->name == "sqrt");
}

void test_anonymous_type_in_subroutine_decl()
{
    Branch branch;
    //branch.eval("def myfunc([int] a) : int\nend");
    // TODO
}

void test_namespace()
{
    Branch branch;
    Term* ns = branch.eval("namespace ns; a = 1; b = 2; end");

    test_assert(branch);
    test_assert(ns->type == NAMESPACE_TYPE);
    test_assert(as_branch(ns).contains("a"));
    test_assert(as_branch(ns).contains("b"));

    Term* a = branch.eval("ns.a");
    test_assert(a->asInt() == 1);

    branch.clear();
    ns = branch.eval("namespace ns; def myfunc(int a) : int; return a+1; end; end");
    Term* c = branch.eval("c = ns.myfunc(4)");
    test_assert(branch);
    test_assert(c->asInt() == 5);

    branch.clear();
    ns = branch.eval("namespace ns1; namespace ns2; x = 12; end; end");
    Term* x = branch.eval("ns1.ns2.x");
    test_assert(branch);
    test_assert(x->asInt() == 12);
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
    REGISTER_TEST_CASE(parser_tests::test_infix_whitespace);
    REGISTER_TEST_CASE(parser_tests::test_list_arguments);
    REGISTER_TEST_CASE(parser_tests::test_function_decl_parse_error);
    REGISTER_TEST_CASE(parser_tests::test_semicolon_as_line_ending);
    REGISTER_TEST_CASE(parser_tests::test_unary_minus);
    REGISTER_TEST_CASE(parser_tests::test_array_index_access);
    REGISTER_TEST_CASE(parser_tests::test_float_division);
    REGISTER_TEST_CASE(parser_tests::test_integer_division);
    REGISTER_TEST_CASE(parser_tests::test_literal_list);
    REGISTER_TEST_CASE(parser_tests::test_anonymous_type_in_subroutine_decl);
    REGISTER_TEST_CASE(parser_tests::test_namespace);
}

} // namespace parser_tests
} // namespace circa
