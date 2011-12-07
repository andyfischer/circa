// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa.h"

namespace token = circa::token;

namespace circa {
namespace tokenizer_tests {

void test_identifiers()
{
    TokenStream tokens("word has_underscore has_hyphen,hasnumbers183,has:colon");

    test_assert(tokens.nextIs(token::IDENTIFIER));
    test_assert(tokens.nextStr() == "word");
    tokens.consume();
    test_assert(tokens.nextIs(token::WHITESPACE));
    tokens.consume();
    test_assert(tokens.nextIs(token::IDENTIFIER));
    test_assert(tokens.nextStr() == "has_underscore");
    tokens.consume();
    test_assert(tokens.nextIs(token::WHITESPACE));
    tokens.consume();
    test_assert(tokens.nextIs(token::IDENTIFIER));
    test_assert(tokens.nextStr() == "has_hyphen");
    tokens.consume();
    test_assert(tokens.nextIs(token::COMMA));
    tokens.consume();
    test_assert(tokens.nextIs(token::IDENTIFIER));
    test_assert(tokens.nextStr() == "hasnumbers183");
    tokens.consume();
    test_assert(tokens.nextIs(token::COMMA));
    tokens.consume();
    test_assert(tokens.nextIs(token::IDENTIFIER));
    test_assert(tokens.nextStr() == "has:colon");
    tokens.consume();
    test_assert(tokens.finished());
}

void test_integers()
{
    TokenStream tokens("1 0 1234567890 0x123");

    test_assert(tokens.nextIs(token::INTEGER));
    test_assert(tokens.nextStr() == "1");
    tokens.consume();
    test_assert(tokens.nextIs(token::WHITESPACE));
    tokens.consume();
    test_assert(tokens.nextIs(token::INTEGER));
    test_assert(tokens.nextStr() == "0");
    tokens.consume();
    test_assert(tokens.nextIs(token::WHITESPACE));
    tokens.consume();
    test_assert(tokens.nextIs(token::INTEGER));
    test_assert(tokens.nextStr() == "1234567890");
    tokens.consume();
    test_assert(tokens.nextIs(token::WHITESPACE));
    tokens.consume();
    test_assert(tokens.nextIs(token::HEX_INTEGER));
    test_assert(tokens.nextStr() == "0x123");
    tokens.consume();
    test_assert(tokens.finished());
}

void test_floats()
{
    TokenStream tokens("1.0 16. .483 .123.");

    test_assert(tokens.nextIs(token::FLOAT_TOKEN));
    test_assert(tokens.nextStr() == "1.0");
    tokens.consume();
    test_assert(tokens.nextIs(token::WHITESPACE));
    tokens.consume();
    test_assert(tokens.nextIs(token::FLOAT_TOKEN));
    test_assert(tokens.nextStr() == "16.");
    tokens.consume();
    test_assert(tokens.nextIs(token::WHITESPACE));
    tokens.consume();
    test_assert(tokens.nextIs(token::FLOAT_TOKEN));
    test_assert(tokens.nextStr() == ".483");
    tokens.consume();
    test_assert(tokens.nextIs(token::WHITESPACE));
    tokens.consume();
    test_assert(tokens.nextIs(token::FLOAT_TOKEN));
    test_assert(tokens.nextStr() == ".123");
    tokens.consume();
    test_assert(tokens.nextIs(token::DOT));
    tokens.consume();
    test_assert(tokens.finished());

    tokens.reset("5.200");

    test_assert(tokens.nextIs(token::FLOAT_TOKEN));
    test_assert(tokens.nextStr() == "5.200");
    tokens.consume();
    test_assert(tokens.finished());

    // Make sure that it ignores two dots. There once was a bug where
    // 0..1 would get parsed as 0. and then .1
    tokens.reset("0..1");
    test_assert(tokens.nextIs(token::INTEGER));
    test_assert(tokens.nextStr() == "0");
    tokens.consume();
    test_assert(tokens.nextIs(token::TWO_DOTS));
    tokens.consume();
    test_assert(tokens.nextIs(token::INTEGER));
    test_assert(tokens.nextStr() == "1");
    tokens.consume();
    test_assert(tokens.finished());
}

void test_misc1()
{
    TokenStream tokens(",()=?][<=>=");

    test_assert(tokens.nextIs(token::COMMA));
    tokens.consume();
    test_assert(tokens.nextIs(token::LPAREN));
    tokens.consume();
    test_assert(tokens.nextIs(token::RPAREN));
    tokens.consume();
    test_assert(tokens.nextIs(token::EQUALS));
    tokens.consume();
    test_assert(tokens.nextIs(token::QUESTION));
    tokens.consume();
    test_assert(tokens.nextIs(token::RBRACKET));
    tokens.consume();
    test_assert(tokens.nextIs(token::LBRACKET));
    tokens.consume();
    test_assert(tokens.nextIs(token::LTHANEQ));
    tokens.consume();
    test_assert(tokens.nextIs(token::GTHANEQ));
    tokens.consume();
    test_assert(tokens.finished());
}

void test_misc2()
{
    TokenStream tokens("<>:;%...<-//&");
    tokens.consume(token::LTHAN);
    tokens.consume(token::GTHAN);
    tokens.consume(token::COLON);
    tokens.consume(token::SEMICOLON);
    tokens.consume(token::PERCENT);
    tokens.consume(token::ELLIPSIS);
    tokens.consume(token::LEFT_ARROW);
    tokens.consume(token::DOUBLE_SLASH);
    tokens.consume(token::AMPERSAND);
    tokens.finished();
}

void test_misc3()
{
    TokenStream tokens("&&!=..::");
    tokens.consume(token::DOUBLE_AMPERSAND);
    tokens.consume(token::NOT_EQUALS);
    tokens.consume(token::TWO_DOTS);
    tokens.consume(token::DOUBLE_COLON);
    test_assert(tokens.finished());
}

void test_keywords()
{
    TokenStream tokens("end,if,else,for,state,do once,elif");

    tokens.consume(token::END);
    tokens.consume(token::COMMA);
    tokens.consume(token::IF);
    tokens.consume(token::COMMA);
    tokens.consume(token::ELSE);
    tokens.consume(token::COMMA);
    tokens.consume(token::FOR);
    tokens.consume(token::COMMA);
    tokens.consume(token::STATE);
    tokens.consume(token::COMMA);
    tokens.consume(token::DO_ONCE);
    tokens.consume(token::COMMA);
    tokens.consume(token::ELIF);
    test_assert(tokens.finished());
}

void test_keywords2()
{
    TokenStream tokens("and or discard return null switch case");

    tokens.consume(token::AND);
    tokens.consume(token::WHITESPACE);
    tokens.consume(token::OR);
    tokens.consume(token::WHITESPACE);
    tokens.consume(token::DISCARD);
    tokens.consume(token::WHITESPACE);
    tokens.consume(token::RETURN);
    tokens.consume(token::WHITESPACE);
    tokens.consume(token::NULL_TOKEN);
    tokens.consume(token::WHITESPACE);
    tokens.consume(token::SWITCH);
    tokens.consume(token::WHITESPACE);
    tokens.consume(token::CASE);
    test_assert(tokens.finished());
}

void test_identifiers_that_look_like_keywords()
{
    TokenStream tokens("endup,iffy,else_,stateful");

    test_equals(tokens.nextStr(), "endup");
    tokens.consume(token::IDENTIFIER);
    tokens.consume(token::COMMA);
    test_equals(tokens.nextStr(), "iffy");
    tokens.consume(token::IDENTIFIER);
    tokens.consume(token::COMMA);
    test_equals(tokens.nextStr(), "else_");
    tokens.consume(token::IDENTIFIER);
    tokens.consume(token::COMMA);
    test_equals(tokens.nextStr(), "stateful");
    tokens.consume(token::IDENTIFIER);
    test_assert(tokens.finished());
}

void test_string_literal()
{
    TokenStream tokens("\"string literal\"'string2'");

    test_equals(tokens.nextStr(), "\"string literal\"");
    tokens.consume(token::STRING);
    test_equals(tokens.nextStr(), "'string2'");
    tokens.consume(token::STRING);
    test_assert(tokens.finished());
}

void test_triple_quote_string_literal()
{
    TokenStream tokens("<<<a string>>>");
    test_equals(tokens.consumeStr(token::STRING), "<<<a string>>>");
    test_assert(tokens.finished());

    tokens.reset("<<< hi > >> >>> >>>");
    test_equals(tokens.consumeStr(token::STRING), "<<< hi > >> >>>");
    tokens.consume(token::WHITESPACE);
    tokens.consume(token::GTHAN);
    tokens.consume(token::GTHAN);
    tokens.consume(token::GTHAN);
    test_assert(tokens.finished());
}

void test_token_stream()
{
    TokenStream tstream("1 2.0");

    test_assert(tstream.nextIs(token::INTEGER));
    test_assert(tstream.nextIs(token::WHITESPACE, 1));
    test_assert(tstream.nextNonWhitespaceIs(token::FLOAT_TOKEN, 1));
}

void test_consume_line()
{
    TokenStream tokens("for in $#!$#@ 151 poop \nfin");

    // happily consume 'for' and some whitespace
    test_assert(tokens.nextIs(token::FOR));
    tokens.consume();
    test_assert(tokens.nextIs(token::WHITESPACE));
    tokens.consume();

    int startPosition = tokens.getPosition();

    // happily consume some more stuff
    test_assert(tokens.nextIs(token::IN_TOKEN));
    tokens.consume();
    test_assert(tokens.nextIs(token::WHITESPACE));
    tokens.consume();

    // now freak out
    std::string errorline = parser::consume_line(tokens, startPosition);

    test_equals(errorline, "in $#!$#@ 151 poop ");
    test_assert(tokens.nextIs(token::IDENTIFIER));
    test_equals(tokens.nextStr(), "fin");
}

void test_color_literal()
{
    TokenStream tokens("#faf");
    test_equals(tokens.consumeStr(token::COLOR), "#faf");
    test_assert(tokens.finished());

    tokens.reset("#119f");
    test_equals(tokens.consumeStr(token::COLOR), "#119f");
    test_assert(tokens.finished());

    tokens.reset("#ff1100");
    test_equals(tokens.consumeStr(token::COLOR), "#ff1100");

    tokens.reset("#00112299");
    test_equals(tokens.consumeStr(token::COLOR), "#00112299");
    test_assert(tokens.finished());

    // test wrong # of characters
    tokens.reset("#00111");
    test_equals(tokens.consumeStr(token::UNRECOGNIZED), "#00111");
    test_assert(tokens.finished());
}

void test_keyword_followed_by_lparen()
{
    TokenStream tokens("if(");

    test_equals(tokens.consumeStr(token::IDENTIFIER), "if");
    tokens.consume(token::LPAREN);
    test_assert(tokens.finished());

    tokens.reset("if (");
    tokens.consume(token::IF);
    tokens.consume(token::WHITESPACE);
    tokens.consume(token::LPAREN);
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
    test_equals(tokens.consumeStr(token::INTEGER), "1");
    test_equals(tokens.consumeStr(token::WHITESPACE), " ");
    test_equals(tokens.consumeStr(token::INTEGER), "2");
    test_equals(tokens.consumeStr(token::WHITESPACE), " ");
    test_equals(tokens.consumeStr(token::COMMENT), "-- this is a comment");
    test_assert(tokens.finished());

    tokens.reset("1 2 # this is a comment");
    test_equals(tokens.consumeStr(token::INTEGER), "1");
    test_equals(tokens.consumeStr(token::WHITESPACE), " ");
    test_equals(tokens.consumeStr(token::INTEGER), "2");
    test_equals(tokens.consumeStr(token::WHITESPACE), " ");
    test_equals(tokens.consumeStr(token::COMMENT), "# this is a comment");
    test_assert(tokens.finished());
}

void test_symbols()
{
    TokenStream tokens(":abc :a :1");
    test_equals(tokens.consumeStr(token::SYMBOL), ":abc");
    test_equals(tokens.consumeStr(token::WHITESPACE), " ");
    test_equals(tokens.consumeStr(token::SYMBOL), ":a");
    test_equals(tokens.consumeStr(token::WHITESPACE), " ");
    test_equals(tokens.consumeStr(token::COLON), ":");
    test_equals(tokens.consumeStr(token::INTEGER), "1");
    test_assert(tokens.finished());
}

void test_number_followed_by_dot_call()
{
    TokenStream tokens("1.something");
    test_equals(tokens.consumeStr(token::INTEGER), "1");
    test_equals(tokens.consumeStr(token::DOT), ".");
    test_equals(tokens.consumeStr(token::IDENTIFIER), "something");
    test_assert(tokens.finished());
}

void register_tests()
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
    REGISTER_TEST_CASE(tokenizer_tests::test_consume_line);
    REGISTER_TEST_CASE(tokenizer_tests::test_color_literal);
    REGISTER_TEST_CASE(tokenizer_tests::test_keyword_followed_by_lparen);
    REGISTER_TEST_CASE(tokenizer_tests::test_preceding_indent);
    REGISTER_TEST_CASE(tokenizer_tests::test_comment);
    REGISTER_TEST_CASE(tokenizer_tests::test_symbols);
    REGISTER_TEST_CASE(tokenizer_tests::test_number_followed_by_dot_call);
}

} // namespace tokenizer_tests

} // namespace circa
