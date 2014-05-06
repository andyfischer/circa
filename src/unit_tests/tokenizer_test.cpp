// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "token.h"

namespace tokenizer_test {

std::string nextStr(TokenStream& tokens)
{
    Value str;
    tokens.getNextStr(&str);
    return as_cstring(&str);
}

void test_identifiers()
{
    TokenStream tokens("word has_underscore has_hyphen,hasnumbers183,has:colon");

    test_assert(tokens.nextIs(tok_Identifier));
    test_assert(nextStr(tokens) == "word");
    tokens.consume();
    test_assert(tokens.nextIs(tok_Whitespace));
    tokens.consume();
    test_assert(tokens.nextIs(tok_Identifier));
    test_assert(nextStr(tokens) == "has_underscore");
    tokens.consume();
    test_assert(tokens.nextIs(tok_Whitespace));
    tokens.consume();
    test_assert(tokens.nextIs(tok_Identifier));
    test_assert(nextStr(tokens) == "has_hyphen");
    tokens.consume();
    test_assert(tokens.nextIs(tok_Comma));
    tokens.consume();
    test_assert(tokens.nextIs(tok_Identifier));
    test_assert(nextStr(tokens) == "hasnumbers183");
    tokens.consume();
    test_assert(tokens.nextIs(tok_Comma));
    tokens.consume();
    test_assert(tokens.nextIs(tok_Identifier));
    test_assert(nextStr(tokens) == "has:colon");
    tokens.consume();
    test_assert(tokens.finished());
}

void test_integers()
{
    TokenStream tokens("1 0 1234567890 0x123");

    test_assert(tokens.nextIs(tok_Integer));
    test_assert(nextStr(tokens) == "1");
    tokens.consume();
    test_assert(tokens.nextIs(tok_Whitespace));
    tokens.consume();
    test_assert(tokens.nextIs(tok_Integer));
    test_assert(nextStr(tokens) == "0");
    tokens.consume();
    test_assert(tokens.nextIs(tok_Whitespace));
    tokens.consume();
    test_assert(tokens.nextIs(tok_Integer));
    test_assert(nextStr(tokens) == "1234567890");
    tokens.consume();
    test_assert(tokens.nextIs(tok_Whitespace));
    tokens.consume();
    test_assert(tokens.nextIs(tok_HexInteger));
    test_assert(nextStr(tokens) == "0x123");
    tokens.consume();
    test_assert(tokens.finished());
}

void test_floats()
{
    TokenStream tokens("1.0 16. .483 .123.");

    test_assert(tokens.nextIs(tok_Float));
    test_assert(nextStr(tokens) == "1.0");
    tokens.consume();
    test_assert(tokens.nextIs(tok_Whitespace));
    tokens.consume();
    test_assert(tokens.nextIs(tok_Float));
    test_assert(nextStr(tokens) == "16.");
    tokens.consume();
    test_assert(tokens.nextIs(tok_Whitespace));
    tokens.consume();
    test_assert(tokens.nextIs(tok_Float));
    test_assert(nextStr(tokens) == ".483");
    tokens.consume();
    test_assert(tokens.nextIs(tok_Whitespace));
    tokens.consume();
    test_assert(tokens.nextIs(tok_Float));
    test_assert(nextStr(tokens) == ".123");
    tokens.consume();
    test_assert(tokens.nextIs(tok_Dot));
    tokens.consume();
    test_assert(tokens.finished());

    tokens.reset("5.200");

    test_assert(tokens.nextIs(tok_Float));
    test_equals(nextStr(tokens), "5.200");
    tokens.consume();
    test_assert(tokens.finished());

    // Make sure that it ignores two dots. There once was a bug where
    // 0..1 would get parsed as 0. and then .1
    tokens.reset("0..1");
    test_assert(tokens.nextIs(tok_Integer));
    test_assert(nextStr(tokens) == "0");
    tokens.consume();
    test_assert(tokens.nextIs(tok_TwoDots));
    tokens.consume();
    test_assert(tokens.nextIs(tok_Integer));
    test_assert(nextStr(tokens) == "1");
    tokens.consume();
    test_assert(tokens.finished());
}

void test_misc1()
{
    TokenStream tokens(",()=?][<=>=");

    test_assert(tokens.nextIs(tok_Comma));
    tokens.consume();
    test_assert(tokens.nextIs(tok_LParen));
    tokens.consume();
    test_assert(tokens.nextIs(tok_RParen));
    tokens.consume();
    test_assert(tokens.nextIs(tok_Equals));
    tokens.consume();
    test_assert(tokens.nextIs(tok_Question));
    tokens.consume();
    test_assert(tokens.nextIs(tok_RSquare));
    tokens.consume();
    test_assert(tokens.nextIs(tok_LSquare));
    tokens.consume();
    test_assert(tokens.nextIs(tok_LThanEq));
    tokens.consume();
    test_assert(tokens.nextIs(tok_GThanEq));
    tokens.consume();
    test_assert(tokens.finished());
}

void test_misc2()
{
    TokenStream tokens("<>:;%...<-//&");
    tokens.consume(tok_LThan);
    tokens.consume(tok_GThan);
    tokens.consume(tok_Colon);
    tokens.consume(tok_Semicolon);
    tokens.consume(tok_Percent);
    tokens.consume(tok_Ellipsis);
    tokens.consume(tok_LeftArrow);
    tokens.consume(tok_DoubleSlash);
    tokens.consume(tok_Ampersand);
    tokens.finished();
}

void test_misc3()
{
    TokenStream tokens("&&!=..::@.@|");
    tokens.consume(tok_DoubleAmpersand);
    tokens.consume(tok_NotEquals);
    tokens.consume(tok_TwoDots);
    tokens.consume(tok_DoubleColon);
    tokens.consume(tok_At);
    tokens.consume(tok_DotAt);
    tokens.consume(tok_VerticalBar);
    test_assert(tokens.finished());
}

void test_stars()
{
    TokenStream tokens("* **");
    tokens.consume(tok_Star);
    tokens.consume(tok_Whitespace);
    tokens.consume(tok_DoubleStar);
    test_assert(tokens.finished());
}

void test_keywords()
{
    TokenStream tokens("if,else,for,state,elif");

    tokens.consume(tok_If);
    tokens.consume(tok_Comma);
    tokens.consume(tok_Else);
    tokens.consume(tok_Comma);
    tokens.consume(tok_For);
    tokens.consume(tok_Comma);
    tokens.consume(tok_State);
    tokens.consume(tok_Comma);
    tokens.consume(tok_Elif);
    test_assert(tokens.finished());
}

void test_keywords2()
{
    TokenStream tokens("and or discard return null switch case");

    tokens.consume(tok_And);
    tokens.consume(tok_Whitespace);
    tokens.consume(tok_Or);
    tokens.consume(tok_Whitespace);
    tokens.consume(tok_Discard);
    tokens.consume(tok_Whitespace);
    tokens.consume(tok_Return);
    tokens.consume(tok_Whitespace);
    tokens.consume(tok_Null);
    tokens.consume(tok_Whitespace);
    tokens.consume(tok_Switch);
    tokens.consume(tok_Whitespace);
    tokens.consume(tok_Case);
    test_assert(tokens.finished());
}

void test_keywords3()
{
    TokenStream tokens("package require section");

    tokens.consume(tok_Package);
    tokens.consume(tok_Whitespace);
    tokens.consume(tok_Require);
    tokens.consume(tok_Whitespace);
    tokens.consume(tok_Section);
}

void test_identifiers_that_look_like_keywords()
{
    TokenStream tokens("endup,iffy,else_,stateful");

    test_equals(nextStr(tokens), "endup");
    tokens.consume(tok_Identifier);
    tokens.consume(tok_Comma);
    test_equals(nextStr(tokens), "iffy");
    tokens.consume(tok_Identifier);
    tokens.consume(tok_Comma);
    test_equals(nextStr(tokens), "else_");
    tokens.consume(tok_Identifier);
    tokens.consume(tok_Comma);
    test_equals(nextStr(tokens), "stateful");
    tokens.consume(tok_Identifier);
    test_assert(tokens.finished());
}

void test_string_literal()
{
    TokenStream tokens("\"string literal\"'string2'");

    test_equals(nextStr(tokens), "\"string literal\"");
    tokens.consume(tok_String);
    test_equals(nextStr(tokens), "'string2'");
    tokens.consume(tok_String);
    test_assert(tokens.finished());

    tokens.reset(":abc :a :1");
    test_equals(tokens.consumeStr(tok_ColonString), ":abc");
    test_equals(tokens.consumeStr(tok_Whitespace), " ");
    test_equals(tokens.consumeStr(tok_ColonString), ":a");
    test_equals(tokens.consumeStr(tok_Whitespace), " ");
    test_equals(tokens.consumeStr(tok_Colon), ":");
    test_equals(tokens.consumeStr(tok_Integer), "1");
    test_assert(tokens.finished());
}

void test_triple_quote_string_literal()
{
    TokenStream tokens("<<<a string>>>");
    test_equals(tokens.consumeStr(tok_String), "<<<a string>>>");
    test_assert(tokens.finished());

    tokens.reset("<<< hi > >> >>> >>>");
    test_equals(tokens.consumeStr(tok_String), "<<< hi > >> >>>");
    tokens.consume(tok_Whitespace);
    tokens.consume(tok_GThan);
    tokens.consume(tok_GThan);
    tokens.consume(tok_GThan);
    test_assert(tokens.finished());
}

void test_color_literal()
{
    TokenStream tokens("#faf");
    test_equals(tokens.consumeStr(tok_Color), "#faf");
    test_assert(tokens.finished());

    tokens.reset("#119f");
    test_equals(tokens.consumeStr(tok_Color), "#119f");
    test_assert(tokens.finished());

    tokens.reset("#ff1100");
    test_equals(tokens.consumeStr(tok_Color), "#ff1100");

    tokens.reset("#00112299");
    test_equals(tokens.consumeStr(tok_Color), "#00112299");
    test_assert(tokens.finished());

    // test wrong # of characters
    tokens.reset("#00111");
    test_equals(tokens.consumeStr(tok_Unrecognized), "#00111");
    test_assert(tokens.finished());
}

void test_keyword_followed_by_lparen()
{
    TokenStream tokens("if(");

    test_equals(tokens.consumeStr(tok_Identifier), "if");
    tokens.consume(tok_LParen);
    test_assert(tokens.finished());

    tokens.reset("if (");
    tokens.consume(tok_If);
    tokens.consume(tok_Whitespace);
    tokens.consume(tok_LParen);
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
        std::string const& txt = nextStr(tokens);
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
    test_equals(tokens.consumeStr(tok_Integer), "1");
    test_equals(tokens.consumeStr(tok_Whitespace), " ");
    test_equals(tokens.consumeStr(tok_Integer), "2");
    test_equals(tokens.consumeStr(tok_Whitespace), " ");
    test_equals(tokens.consumeStr(tok_Comment), "-- this is a comment");
    test_assert(tokens.finished());
}

void test_number_followed_by_dot_call()
{
    TokenStream tokens("1.something");
    test_equals(tokens.consumeStr(tok_Integer), "1");
    test_equals(tokens.consumeStr(tok_Dot), ".");
    test_equals(tokens.consumeStr(tok_Identifier), "something");
    test_assert(tokens.finished());
}


void register_tests()
{
    REGISTER_TEST_CASE(tokenizer_test::test_identifiers);
    REGISTER_TEST_CASE(tokenizer_test::test_integers);
    REGISTER_TEST_CASE(tokenizer_test::test_floats);
    REGISTER_TEST_CASE(tokenizer_test::test_misc1);
    REGISTER_TEST_CASE(tokenizer_test::test_misc2);
    REGISTER_TEST_CASE(tokenizer_test::test_misc3);
    REGISTER_TEST_CASE(tokenizer_test::test_stars);
    REGISTER_TEST_CASE(tokenizer_test::test_keywords);
    REGISTER_TEST_CASE(tokenizer_test::test_keywords2);
    REGISTER_TEST_CASE(tokenizer_test::test_keywords3);
    REGISTER_TEST_CASE(tokenizer_test::test_identifiers_that_look_like_keywords);
    REGISTER_TEST_CASE(tokenizer_test::test_string_literal);
    REGISTER_TEST_CASE(tokenizer_test::test_triple_quote_string_literal);
    REGISTER_TEST_CASE(tokenizer_test::test_color_literal);
    REGISTER_TEST_CASE(tokenizer_test::test_keyword_followed_by_lparen);
    REGISTER_TEST_CASE(tokenizer_test::test_preceding_indent);
    REGISTER_TEST_CASE(tokenizer_test::test_comment);
    REGISTER_TEST_CASE(tokenizer_test::test_number_followed_by_dot_call);
}

} // namespace 
