// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "framework.h"
#include "token.h"

using namespace circa;

namespace tokenizer_tests {

void test_identifiers()
{
    TokenStream tokens("word has_underscore has_hyphen,hasnumbers183,has:colon");

    test_assert(tokens.nextIs(TK_IDENTIFIER));
    test_assert(tokens.nextStr() == "word");
    tokens.consume();
    test_assert(tokens.nextIs(TK_WHITESPACE));
    tokens.consume();
    test_assert(tokens.nextIs(TK_IDENTIFIER));
    test_assert(tokens.nextStr() == "has_underscore");
    tokens.consume();
    test_assert(tokens.nextIs(TK_WHITESPACE));
    tokens.consume();
    test_assert(tokens.nextIs(TK_IDENTIFIER));
    test_assert(tokens.nextStr() == "has_hyphen");
    tokens.consume();
    test_assert(tokens.nextIs(TK_COMMA));
    tokens.consume();
    test_assert(tokens.nextIs(TK_IDENTIFIER));
    test_assert(tokens.nextStr() == "hasnumbers183");
    tokens.consume();
    test_assert(tokens.nextIs(TK_COMMA));
    tokens.consume();
    test_assert(tokens.nextIs(TK_IDENTIFIER));
    test_assert(tokens.nextStr() == "has:colon");
    tokens.consume();
    test_assert(tokens.finished());
}

void test_integers()
{
    TokenStream tokens("1 0 1234567890 0x123");

    test_assert(tokens.nextIs(TK_INTEGER));
    test_assert(tokens.nextStr() == "1");
    tokens.consume();
    test_assert(tokens.nextIs(TK_WHITESPACE));
    tokens.consume();
    test_assert(tokens.nextIs(TK_INTEGER));
    test_assert(tokens.nextStr() == "0");
    tokens.consume();
    test_assert(tokens.nextIs(TK_WHITESPACE));
    tokens.consume();
    test_assert(tokens.nextIs(TK_INTEGER));
    test_assert(tokens.nextStr() == "1234567890");
    tokens.consume();
    test_assert(tokens.nextIs(TK_WHITESPACE));
    tokens.consume();
    test_assert(tokens.nextIs(TK_HEX_INTEGER));
    test_assert(tokens.nextStr() == "0x123");
    tokens.consume();
    test_assert(tokens.finished());
}

void test_floats()
{
    TokenStream tokens("1.0 16. .483 .123.");

    test_assert(tokens.nextIs(TK_FLOAT));
    test_assert(tokens.nextStr() == "1.0");
    tokens.consume();
    test_assert(tokens.nextIs(TK_WHITESPACE));
    tokens.consume();
    test_assert(tokens.nextIs(TK_FLOAT));
    test_assert(tokens.nextStr() == "16.");
    tokens.consume();
    test_assert(tokens.nextIs(TK_WHITESPACE));
    tokens.consume();
    test_assert(tokens.nextIs(TK_FLOAT));
    test_assert(tokens.nextStr() == ".483");
    tokens.consume();
    test_assert(tokens.nextIs(TK_WHITESPACE));
    tokens.consume();
    test_assert(tokens.nextIs(TK_FLOAT));
    test_assert(tokens.nextStr() == ".123");
    tokens.consume();
    test_assert(tokens.nextIs(TK_DOT));
    tokens.consume();
    test_assert(tokens.finished());

    tokens.reset("5.200");

    test_assert(tokens.nextIs(TK_FLOAT));
    test_assert(tokens.nextStr() == "5.200");
    tokens.consume();
    test_assert(tokens.finished());

    // Make sure that it ignores two dots. There once was a bug where
    // 0..1 would get parsed as 0. and then .1
    tokens.reset("0..1");
    test_assert(tokens.nextIs(TK_INTEGER));
    test_assert(tokens.nextStr() == "0");
    tokens.consume();
    test_assert(tokens.nextIs(TK_TWO_DOTS));
    tokens.consume();
    test_assert(tokens.nextIs(TK_INTEGER));
    test_assert(tokens.nextStr() == "1");
    tokens.consume();
    test_assert(tokens.finished());
}

void test_misc1()
{
    TokenStream tokens(",()=?][<=>=");

    test_assert(tokens.nextIs(TK_COMMA));
    tokens.consume();
    test_assert(tokens.nextIs(TK_LPAREN));
    tokens.consume();
    test_assert(tokens.nextIs(TK_RPAREN));
    tokens.consume();
    test_assert(tokens.nextIs(TK_EQUALS));
    tokens.consume();
    test_assert(tokens.nextIs(TK_QUESTION));
    tokens.consume();
    test_assert(tokens.nextIs(TK_RBRACKET));
    tokens.consume();
    test_assert(tokens.nextIs(TK_LBRACKET));
    tokens.consume();
    test_assert(tokens.nextIs(TK_LTHANEQ));
    tokens.consume();
    test_assert(tokens.nextIs(TK_GTHANEQ));
    tokens.consume();
    test_assert(tokens.finished());
}

void test_misc2()
{
    TokenStream tokens("<>:;%...<-//&");
    tokens.consume(TK_LTHAN);
    tokens.consume(TK_GTHAN);
    tokens.consume(TK_COLON);
    tokens.consume(TK_SEMICOLON);
    tokens.consume(TK_PERCENT);
    tokens.consume(TK_ELLIPSIS);
    tokens.consume(TK_LEFT_ARROW);
    tokens.consume(TK_DOUBLE_SLASH);
    tokens.consume(TK_AMPERSAND);
    tokens.finished();
}

void test_misc3()
{
    TokenStream tokens("&&!=..::@..@");
    tokens.consume(TK_DOUBLE_AMPERSAND);
    tokens.consume(TK_NOT_EQUALS);
    tokens.consume(TK_TWO_DOTS);
    tokens.consume(TK_DOUBLE_COLON);
    tokens.consume(TK_AT_DOT);
    tokens.consume(TK_DOT);
    tokens.consume(TK_AT_SIGN);
    test_assert(tokens.finished());
}

void test_keywords()
{
    TokenStream tokens("end,if,else,for,state,do once,elif");

    tokens.consume(TK_END);
    tokens.consume(TK_COMMA);
    tokens.consume(TK_IF);
    tokens.consume(TK_COMMA);
    tokens.consume(TK_ELSE);
    tokens.consume(TK_COMMA);
    tokens.consume(TK_FOR);
    tokens.consume(TK_COMMA);
    tokens.consume(TK_STATE);
    tokens.consume(TK_COMMA);
    tokens.consume(TK_DO_ONCE);
    tokens.consume(TK_COMMA);
    tokens.consume(TK_ELIF);
    test_assert(tokens.finished());
}

void test_keywords2()
{
    TokenStream tokens("and or discard return null switch case");

    tokens.consume(TK_AND);
    tokens.consume(TK_WHITESPACE);
    tokens.consume(TK_OR);
    tokens.consume(TK_WHITESPACE);
    tokens.consume(TK_DISCARD);
    tokens.consume(TK_WHITESPACE);
    tokens.consume(TK_RETURN);
    tokens.consume(TK_WHITESPACE);
    tokens.consume(TK_NULL);
    tokens.consume(TK_WHITESPACE);
    tokens.consume(TK_SWITCH);
    tokens.consume(TK_WHITESPACE);
    tokens.consume(TK_CASE);
    test_assert(tokens.finished());
}

void test_identifiers_that_look_like_keywords()
{
    TokenStream tokens("endup,iffy,else_,stateful");

    test_equals(tokens.nextStr(), "endup");
    tokens.consume(TK_IDENTIFIER);
    tokens.consume(TK_COMMA);
    test_equals(tokens.nextStr(), "iffy");
    tokens.consume(TK_IDENTIFIER);
    tokens.consume(TK_COMMA);
    test_equals(tokens.nextStr(), "else_");
    tokens.consume(TK_IDENTIFIER);
    tokens.consume(TK_COMMA);
    test_equals(tokens.nextStr(), "stateful");
    tokens.consume(TK_IDENTIFIER);
    test_assert(tokens.finished());
}

void test_string_literal()
{
    TokenStream tokens("\"string literal\"'string2'");

    test_equals(tokens.nextStr(), "\"string literal\"");
    tokens.consume(TK_STRING);
    test_equals(tokens.nextStr(), "'string2'");
    tokens.consume(TK_STRING);
    test_assert(tokens.finished());
}

void test_triple_quote_string_literal()
{
    TokenStream tokens("<<<a string>>>");
    test_equals(tokens.consumeStr(TK_STRING), "<<<a string>>>");
    test_assert(tokens.finished());

    tokens.reset("<<< hi > >> >>> >>>");
    test_equals(tokens.consumeStr(TK_STRING), "<<< hi > >> >>>");
    tokens.consume(TK_WHITESPACE);
    tokens.consume(TK_GTHAN);
    tokens.consume(TK_GTHAN);
    tokens.consume(TK_GTHAN);
    test_assert(tokens.finished());
}

void test_token_stream()
{
    TokenStream tstream("1 2.0");

    test_assert(tstream.nextIs(TK_INTEGER));
    test_assert(tstream.nextIs(TK_WHITESPACE, 1));
    test_assert(tstream.nextNonWhitespaceIs(TK_FLOAT, 1));
}

void test_color_literal()
{
    TokenStream tokens("#faf");
    test_equals(tokens.consumeStr(TK_COLOR), "#faf");
    test_assert(tokens.finished());

    tokens.reset("#119f");
    test_equals(tokens.consumeStr(TK_COLOR), "#119f");
    test_assert(tokens.finished());

    tokens.reset("#ff1100");
    test_equals(tokens.consumeStr(TK_COLOR), "#ff1100");

    tokens.reset("#00112299");
    test_equals(tokens.consumeStr(TK_COLOR), "#00112299");
    test_assert(tokens.finished());

    // test wrong # of characters
    tokens.reset("#00111");
    test_equals(tokens.consumeStr(TK_UNRECOGNIZED), "#00111");
    test_assert(tokens.finished());
}

void test_keyword_followed_by_lparen()
{
    TokenStream tokens("if(");

    test_equals(tokens.consumeStr(TK_IDENTIFIER), "if");
    tokens.consume(TK_LPAREN);
    test_assert(tokens.finished());

    tokens.reset("if (");
    tokens.consume(TK_IF);
    tokens.consume(TK_WHITESPACE);
    tokens.consume(TK_LPAREN);
    test_assert(tokens.finished());
}

void test_preceding_indent()
{
    TokenStream tokens("1 2 3");
    while (!tokens.finished()) {
        test_assert(tokens.next().precedingIndent == 0);
        tokens.consume();
    }

    tokens.reset("  1 2");
    while (!tokens.finished()) {
        test_assert(tokens.next().precedingIndent == 2);
        tokens.consume();
    }

    tokens.reset("a a1 a2\n"
                 "   b1 b2\n"
                 " c1 c2\n"
                 "      d1 + d2\n");

    while (!tokens.finished()) {
        std::string const& txt = tokens.nextStr();
        if (txt[0] == 'a') {
            test_assert(tokens.next().precedingIndent == 0);
        } else if (txt[0] == 'b') {
            test_assert(tokens.next().precedingIndent == 3);
        } else if (txt[0] == 'c') {
            test_assert(tokens.next().precedingIndent == 1);
        } else if (txt[0] == 'd') {
            test_assert(tokens.next().precedingIndent == 6);
        }
        tokens.consume();
    }
}

void test_comment()
{
    TokenStream tokens("1 2 -- this is a comment");
    test_equals(tokens.consumeStr(TK_INTEGER), "1");
    test_equals(tokens.consumeStr(TK_WHITESPACE), " ");
    test_equals(tokens.consumeStr(TK_INTEGER), "2");
    test_equals(tokens.consumeStr(TK_WHITESPACE), " ");
    test_equals(tokens.consumeStr(TK_COMMENT), "-- this is a comment");
    test_assert(tokens.finished());
}

void test_names()
{
    TokenStream tokens(":abc :a :1");
    test_equals(tokens.consumeStr(TK_NAME), ":abc");
    test_equals(tokens.consumeStr(TK_WHITESPACE), " ");
    test_equals(tokens.consumeStr(TK_NAME), ":a");
    test_equals(tokens.consumeStr(TK_WHITESPACE), " ");
    test_equals(tokens.consumeStr(TK_COLON), ":");
    test_equals(tokens.consumeStr(TK_INTEGER), "1");
    test_assert(tokens.finished());
}

void test_number_followed_by_dot_call()
{
    TokenStream tokens("1.something");
    test_equals(tokens.consumeStr(TK_INTEGER), "1");
    test_equals(tokens.consumeStr(TK_DOT), ".");
    test_equals(tokens.consumeStr(TK_IDENTIFIER), "something");
    test_assert(tokens.finished());
}

} // namespace tokenizer_tests

void tokenizer_register_tests()
{
    REGISTER_TEST_CASE(tokenizer_tests::test_identifiers);
    REGISTER_TEST_CASE(tokenizer_tests::test_integers);
    REGISTER_TEST_CASE(tokenizer_tests::test_floats);
    REGISTER_TEST_CASE(tokenizer_tests::test_misc1);
    REGISTER_TEST_CASE(tokenizer_tests::test_misc2);
    REGISTER_TEST_CASE(tokenizer_tests::test_misc3);
    REGISTER_TEST_CASE(tokenizer_tests::test_keywords);
    REGISTER_TEST_CASE(tokenizer_tests::test_keywords2);
    REGISTER_TEST_CASE(tokenizer_tests::test_identifiers_that_look_like_keywords);
    REGISTER_TEST_CASE(tokenizer_tests::test_string_literal);
    REGISTER_TEST_CASE(tokenizer_tests::test_triple_quote_string_literal);
    REGISTER_TEST_CASE(tokenizer_tests::test_token_stream);
    REGISTER_TEST_CASE(tokenizer_tests::test_color_literal);
    REGISTER_TEST_CASE(tokenizer_tests::test_keyword_followed_by_lparen);
    REGISTER_TEST_CASE(tokenizer_tests::test_preceding_indent);
    REGISTER_TEST_CASE(tokenizer_tests::test_comment);
    REGISTER_TEST_CASE(tokenizer_tests::test_names);
    REGISTER_TEST_CASE(tokenizer_tests::test_number_followed_by_dot_call);
}
